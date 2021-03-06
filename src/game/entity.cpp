/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "entity.hpp"
#include "base/defs.hpp"
#include "audio.hpp"
#include "color.hpp"
#include "control.hpp"
#include "defs.hpp"
#include "graphics.hpp"
#include "leveldata.hpp"
#include "stats.hpp"
#include "persistent.hpp"
#include <cstdio>
#include <algorithm>
namespace game {

using ::audio::sfx;
using ::graphics::color;
using ::graphics::sprite;
using ::graphics::ui;
using ::graphics::anysprite;
using ::sprite::orientation;

static const int HIT_TIME = 25;
static const int SHOT_DELAY = 8;

static const float DT = 1e-3 * defs::FRAMETIME;
static const float INV_DT = 1.0 / (1e-3 * defs::FRAMETIME);

static const int CAMERA_X = 16;
static const int CAMERA_Y = 16;
static const frect CAMERA(-CAMERA_X, -CAMERA_Y, +CAMERA_X, +CAMERA_Y);

struct entity_is_dead {
    bool operator()(const std::unique_ptr<entity> &p) {
        return p->m_team != team::DEAD;
    }
};

// ======================================================================

static std::string door_name(const std::string &data)
{
    auto pos = data.rfind(':');
    if (pos == std::string::npos)
        return data;
    return data.substr(pos + 1);
}

static anysprite glyph_sprite(persistent_state &state,
                              const std::string &data)
{
    if (data.empty())
        core::die("Empty glyph data");

    if (data[0] == 't') {
        if (data.size() == 2) {
            int treasure = data[1] - '1';
            if (treasure >= 0 && treasure < 3)
                return ::graphics::treasure_sprite(
                    treasure, state.treasure[treasure]);
        }
    }

    core::die("Invalid glyph data");
}

entity_system::entity_system(persistent_state &state,
                             const control_system &control,
                             audio::system &audio,
                             const std::string &levelname,
                             const std::string &lastlevel)
    : state_(state), control_(control), audio_(audio), levelname_(levelname),
      is_click_(false)
{
    level_.set_level(levelname);
    auto data = leveldata::read_level(levelname);
    auto b = data.begin(), e = data.end();
    spawnpoint *pspawn = nullptr, *dspawn = nullptr, *dspawn2 = nullptr;
    std::string dname;

    for (auto i = b; i != e; i++) {
        switch (i->type) {
        case spawntype::PLAYER:
            pspawn = &*i;
            break;

        case spawntype::DOOR:
            entities_.emplace_back(new door(*this, fvec(i->pos), i->data));
            if (door_name(i->data) == lastlevel)
                dspawn = &*i;
            dspawn2 = &*i;
            break;

        case spawntype::CHEST:
            entities_.emplace_back(new chest(*this, fvec(i->pos), i->data));
            break;

        case spawntype::PROF:
            entities_.emplace_back(
                new enemy(*this, fvec(i->pos),
                          sprite::PROFESSOR, sprite::BOOK1, sprite::BOOK2));
            break;

        case spawntype::WOMAN:
            entities_.emplace_back(
                new enemy(*this, fvec(i->pos),
                          sprite::WOMAN, sprite::MOUTH1, sprite::MOUTH2));
            break;

        case spawntype::PRIEST:
            entities_.emplace_back(
                new enemy(*this, fvec(i->pos),
                          sprite::PRIEST, sprite::SKULL, sprite::SKULL));
            break;

        case spawntype::GLYPH:
            entities_.emplace_back(
                new glyph(*this, fvec(i->pos), glyph_sprite(state, i->data)));
            break;

        case spawntype::MUSIC:
            audio_.play_music(i->data, false);
            break;

        default:
            core::die("Cannot spawn entity, unknown type");
        }
    }

    // Don't care if there is no player, maybe it would be useful.
    if (!lastlevel.empty() && dspawn != nullptr)
        pspawn = dspawn;
    if (pspawn == nullptr && dspawn2 != nullptr) {
        std::puts("Fallback, no suitable door found");
        pspawn = dspawn2;
    }
    if (pspawn != nullptr)
        entities_.emplace_back(
            new player(*this, fvec(pspawn->pos)));

    camera_ = camera_system(
        frect(0.0f, 0.0f, level_.width(), level_.height()));
}

void entity_system::update()
{
    if (state_.hittime > 0)
        state_.hittime--;
    hover_trigger_ = ivec(-1000, -1000);

    entities_.insert(
        entities_.end(),
        std::make_move_iterator(new_entities_.begin()),
        std::make_move_iterator(new_entities_.end()));
    new_entities_.clear();
    for (auto i = entities_.begin(), e = entities_.end(); i != e; i++) {
        entity &ent = **i;
        ent.update();
    }
    auto part = std::stable_partition(
        entities_.begin(), entities_.end(), entity_is_dead());
    entities_.erase(part, entities_.end());

    camera_.update();
    is_click_ = false;
}

void entity_system::draw(::graphics::system &gr, int reltime)
{
    color base = color::palette(0).fade(0.5f);
    color hurt = color::palette(6);
    if (state_.hittime > 0) {
        gr.set_blend_color(color::blend(
            base, hurt,
            state_.hittime * (1.0f / HIT_TIME)));
    } else {
        gr.set_blend_color(base);
    }

    for (int i = 0; i < state_.maxhealth; i++) {
        gr.add_sprite(
            i < state_.health ? ui::HEART1 : ui::HEART2,
            ivec(core::PWIDTH - 8 - 16*i, core::PHEIGHT - 8),
            orientation::NORMAL,
            true);
    }

    fvec camera = camera_.get_pos(reltime);
    lastcamera_ = ivec(camera);
    gr.set_camera_pos(lastcamera_);
    for (auto i = entities_.begin(), e = entities_.end(); i != e; i++) {
        entity &ent = **i;
        ent.draw(gr, reltime);
    }
}

void entity_system::add_entity(entity *ent)
{
    if (ent)
        new_entities_.push_back(std::unique_ptr<entity>(ent));
}

void entity_system::set_camera_target(const frect &target)
{
    camera_.set_target(target);
}

void entity_system::set_hover(ivec pos)
{
    hover_trigger_ = pos;
}

bool entity_system::test_hover(irect rect)
{
    return rect.contains(hover_trigger_);
}

entity *entity_system::scan_target(irect range, team t)
{
    for (auto i = entities_.begin(), e = entities_.end(); i != e; i++) {
        entity &ent = **i;
        if (ent.m_team == t && irect::test_intersect(ent.m_bbox, range))
            return &ent;
    }
    return nullptr;
}

void entity_system::mouse_click(ivec pos, int button)
{
    if (button != 1 && button != 3)
        return;
    is_click_ = true;
    click_pos_ = pos + lastcamera_
        - ivec(core::PWIDTH / 2, core::PHEIGHT / 2);
}

void entity_system::spawn_shot(
    team t, fvec origin, fvec target, float speed,
    ::graphics::anysprite sp1, ::graphics::anysprite sp2,
    int delay)
{
    origin += fvec(
        std::copysign(10.0f, target.x - origin.x),
        4.0f);
    fvec delta = target - origin;
    float mag2 = delta.mag2();
    fvec shotvel;
    if (mag2 < 1)
        shotvel = fvec(speed, 0.0f);
    else
        shotvel = delta * (speed / std::sqrt(mag2));
    add_entity(
        new shot(*this, t, origin, shotvel, delay, sp1, sp2));
}

// ======================================================================

entity::entity(entity_system &sys, team t)
    : m_system(sys), m_bbox(0, 0, 0, 0),
      m_team(t)
{ }

entity::~entity()
{ }

void entity::update()
{ }

void entity::interact()
{ }

void entity::damage(int amount)
{
    (void)amount;
}

// ======================================================================

physics_component::physics_component(irect bbox, fvec pos, fvec vel)
    : bbox(bbox), pos(pos), vel(vel), accel(0, -stats::gravity),
      on_floor(false)
{ }

void physics_component::update(entity_system &sys, entity &e)
{
    static const int MAXSTEP = 3;

    on_floor = false;
    lastpos = pos;
    fvec old_pos = pos;
    fvec new_pos = old_pos + DT * vel + (DT * DT / 2) * accel;
    fvec old_vel = vel;
    fvec new_vel = old_vel + DT * accel;

    int x1 = (int)std::floor(new_pos.x);
    int y1 = (int)std::floor(new_pos.y);
    irect new_bbox = bbox.offset(ivec(x1, y1));
    const levelmap &level = sys.level();
    // std::printf("%d %d %d %d\n",
    // new_bbox.x0, new_bbox.y0, new_bbox.x1, new_bbox.y1);
    if (level.hit_test(new_bbox)) {
        // std::puts("===== COLLIDE =====");
        // Scan dx decreasing in magnitude until we can find a spot
        // for the entity.  The entity's vertical position will be
        // adjusted as necessary.
        int x0 = (int)std::floor(old_pos.x);
        int y0 = (int)std::floor(old_pos.y);
        int dir = x1 < x0 ? -1 : +1;
        int max_dx = dir * (x1 - x0);
        if (max_dx < 0)
            max_dx = 0;
        for (int dx = max_dx; ; dx--) {
            int x = x0 + dx * dir;
            int step = dx;
            if (step < MAXSTEP)
                step = MAXSTEP;
            int ymin = y0 - step, ymax = y0 + step;
            if (y1 < ymin) ymin = y1;
            else if (y1 > ymax) ymax = y1;
            int yh = (ymin + ymax) / 2;
            irect rect0(x + bbox.x0, ymin + bbox.y0,
                        x + bbox.x1, yh);
            irect rect1(x + bbox.x0, yh,
                        x + bbox.x1, ymax + bbox.y1);
            int d0 = level.hit_y0(rect0);
            int d1 = level.hit_y1(rect1);
            // std::printf("rects: %d %d %d %d\n",
            // rect0.y0, rect0.y1, rect1.y0, rect1.y1);
            // std::printf("d0: %d, d1: %d\n", d0, d1);
            int miny1 = ymin + d0;
            int maxy1 = ymax - d1;
            // std::printf("min: %d, max: %d, y0: %d, y1: %d\n",
            // miny1, maxy1, y0, y1);
            if (miny1 <= maxy1) {
                if (dx < max_dx)
                    new_pos.x = x;
                if (y1 < miny1) {
                    new_pos.y = miny1;
                    on_floor = true;
                } else if (y1 > maxy1) {
                    new_pos.y = maxy1;
                }
                // std::printf("new: %f, old: %f\n", new_pos.y, old_pos.y);
                break;
            }
            if (dx == 0) {
                std::puts("Entity stuck!");
                new_pos = old_pos;
                break;
            }
        }

        new_vel = (1.0f / DT) * (new_pos - old_pos);
    }

    pos = new_pos;
    vel = new_vel;

    e.m_bbox = new_bbox;
    accel = fvec(0, -stats::gravity);
}

ivec physics_component::get_pos(int reltime)
{
    return ivec(defs::interp(lastpos, pos, reltime));
}

// ======================================================================

projectile_component::projectile_component(
    irect bbox, fvec pos, fvec vel, int damage)
    : bbox(bbox), lastpos(pos), pos(pos), vel(vel),
      damage(damage)
{ }

void projectile_component::update(entity_system &sys, entity &e)
{
    lastpos = pos;
    pos += vel * DT;

    int x1 = (int)std::floor(pos.x);
    int y1 = (int)std::floor(pos.y);
    e.m_bbox = bbox.offset(ivec(x1, y1));

    bool hit_level = sys.level().hit_test(e.m_bbox);
    bool hit_actor = false;

    team enemy;
    switch (e.m_team) {
    default:
    case team::FRIEND_SHOT: enemy = team::FOE; break;
    case team::FOE_SHOT: enemy = team::FRIEND; break;
    }

    auto be = sys.entities().begin(), ee = sys.entities().end();
    for (auto i = be; i != ee; i++) {
        entity &target = **i;
        if (target.m_team != enemy)
            continue;
        if (!irect::test_intersect(target.m_bbox, e.m_bbox))
            continue;
        target.damage(damage);
        hit_actor = true;
    }

    if (hit_level || hit_actor) {
        e.m_team = team::DEAD;
        sys.add_entity(new poof(sys, pos));
        if (!hit_actor)
            sys.audio().play_sfx(sfx::SHOT_IMPACT);
    }
}

ivec projectile_component::get_pos(int reltime)
{
    return ivec(defs::interp(lastpos, pos, reltime));
}

// ======================================================================

walking_component::walking_component()
    : xmove(0.0f), step_distance(0.0f)
{ }

static const float STEP_DISTANCE = 32.0f;
static const float STEP_FALL_SPEED = 50.0f;

void walking_component::update(physics_component &physics,
                               const walking_stats &stats)
{
    bool on_floor = physics.on_floor;
    float max_speed = on_floor ? stats.speed_ground : stats.speed_air;
    float max_accel = on_floor ? stats.accel_ground : stats.accel_air;
    float accel = (max_speed * xmove - physics.vel.x) * INV_DT;
    if (accel > max_accel)
        accel = max_accel;
    else if (accel < -max_accel)
        accel = -max_accel;
    physics.accel.x += accel;
    walk_sound = false;
    if (on_floor) {
        step_distance += std::abs(physics.vel.x * DT);
        if (step_distance > STEP_DISTANCE) {
            step_distance -= STEP_DISTANCE;
            walk_sound = true;
        }
    } else {
        if (physics.vel.y < -STEP_FALL_SPEED) {
            step_distance = STEP_DISTANCE;
        }
    }
}

// ======================================================================

#if 0
static void jump_simple(physics_component &physics, fvec vel)
{
    physics.accel += (vel - physics.vel) * INV_DT;
}
#endif

// ======================================================================

jumping_component::jumping_component()
    : ymove(0.0f), jumptime(0), jstate(jumpstate::READY)
{ }

void jumping_component::update(physics_component &physics,
                               const jumping_stats &stats)
{
    jump_sound = false;
    bool do_jump = false;
    if (physics.on_floor) {
        jumptime = 0;
        if (ymove > 0.5f) {
            if (jstate == jumpstate::READY) {
                jstate = jumpstate::JUMP1;
                do_jump = true;
            }
        } else {
            jstate = jumpstate::READY;
        }
    } else {
        if (ymove >= 0.50f) {
            if (jumptime > 0) {
                jumptime--;
                physics.accel.y += stats.accel * ymove;
            } else if (jstate == jumpstate::READY && stats.can_doublejump) {
                jstate = jumpstate::JUMP2;
                do_jump = true;
            }
        } else {
            jumptime = 0;
            if (jstate == jumpstate::JUMP1)
                jstate = jumpstate::READY;
        }
    }

    if (do_jump) {
        jump_sound = true;
        jumptime = stats.jumptime;
        if (stats.speed > physics.vel.y)
            physics.accel.y +=
                (stats.speed - physics.vel.y) * INV_DT;
    }

    ymove = 0.0f;
}

// ======================================================================

enemy_component::enemy_component()
    : m_state(state::IDLE), m_time(0)
{ }

void enemy_component::update(entity_system &sys, entity &e,
                             const enemy_stats &stats)
{
    entity *target;
    switch (m_state) {
    case state::IDLE:
        target = scan(sys, e, stats);
        if (target != nullptr) {
            m_state = state::ALERT;
            m_time = stats.reaction;
            m_targetpos = target->m_bbox.center();
        }
        break;

    case state::ALERT:
        m_time--;
        if (m_time == 0) {
            target = scan(sys, e, stats);
            if (target != nullptr) {
                m_state = state::ATTACK;
                m_time = 0;
                m_targetpos = target->m_bbox.center();
            } else {
                m_state = state::IDLE;
            }
        }
        break;

    case state::ATTACK:
        m_time++;
        if (m_time >= stats.attacktime) {
            m_state = state::ALERT;
            m_time = stats.interval;
        }
        break;
    }
}

entity *enemy_component::scan(entity_system &sys, entity &e,
                              const enemy_stats &stats)
{
    irect vision = irect::centered(stats.xsight * 2, stats.ysight * 2);
    return sys.scan_target(e.m_bbox.expand(vision), team::FRIEND);
}

// ======================================================================

player::player(entity_system &sys, fvec pos)
    : entity(sys, team::FRIEND),
      physics(irect::centered(8, 20), pos, fvec::zero())
{ }

player::~player()
{ }

void player::update()
{
    walking.xmove = m_system.control().get_xaxis();
    jumping.ymove = m_system.control().get_yaxis();

    walking.update(physics, stats::player_walk);
    jumping.update(physics, stats::player_jump);
    physics.update(m_system, *this);

    if (walking.walk_sound)
        m_system.audio().play_sfx(sfx::PLAYER_STEP);
    if (jumping.jump_sound)
        m_system.audio().play_sfx(sfx::PLAYER_JUMP);

    m_system.set_camera_target(CAMERA.offset(physics.pos));
    m_system.set_hover(ivec(physics.pos));

    if (m_system.is_click()) {
        m_system.audio().play_sfx(sfx::PLAYER_SHOOT);
        auto target = m_system.click_pos();
        m_system.spawn_shot(
            team::FRIEND_SHOT,
            physics.pos,
            fvec(target.x, target.y),
            stats::player_shotspeed,
            sprite::SHOT,
            sprite::SHOT,
            0);
    }

    if (m_system.control().get_key_instant(key::DOWN)) {
        ivec pos(physics.pos);
        auto &ents = m_system.entities();
        for (auto i = ents.begin(), e = ents.end(); i != e; i++) {
            entity &ent(**i);
            if (ent.m_team != team::INTERACTIVE)
                continue;
            if (!ent.m_bbox.contains(pos))
                continue;
            ent.interact();
            break;
        }
    }

    if (physics.pos.y < -50.0f)
        player_die();
}

void player::damage(int amt)
{
    m_system.audio().play_sfx(sfx::PLAYER_HIT);
    auto &s = m_system.state();
    s.hittime += HIT_TIME * amt;
    if (s.maxhealth > 0) {
        s.health -= amt;
        if (s.health <= 0)
            player_die();
    }
}

void player::draw(::graphics::system &gr, int reltime)
{
    gr.add_sprite(
        sprite::PLAYER,
        ivec(physics.get_pos(reltime)),
        orientation::NORMAL);
}

void player::player_die()
{
    m_team = team::DEAD;
    m_system.audio().play_music("die", false);
    m_system.state().hittime = HIT_TIME * 8;
    m_system.is_player_dead = true;
    m_system.add_entity(new signal_glyph(
        m_system, physics.pos, sprite::SKULL, "!dead", true));
}

// ======================================================================

door::door(entity_system &sys, fvec pos, const std::string target)
    : entity(sys, team::INTERACTIVE), m_pos(pos), m_target(target),
      m_is_locked(false)
{
    m_bbox = irect::centered(24, 32).offset(m_pos);
    if (door_name(target) == "!end") {
        for (int i = 0; i < 3; i++) {
            if (sys.state().treasure[i] == 0) {
                m_is_locked = true;
                break;
            }
        }
    }
}

door::~door()
{ }

void door::interact()
{
    if (!m_is_locked) {
        m_system.audio().play_sfx(sfx::DOOR_OPEN);
        m_system.nextlevel = m_target;
    }
}

void door::draw(::graphics::system &gr, int reltime)
{
    (void)reltime;
    gr.add_sprite(
        m_is_locked ? sprite::DOOR3 : sprite::DOOR2,
        m_pos,
        orientation::NORMAL);
    if (!m_is_locked && m_system.test_hover(m_bbox)) {
        gr.add_sprite(
            ui::ARROW,
            m_pos + ivec(0, 28),
            orientation::NORMAL);
    }
}

// ======================================================================

chest::chest(entity_system &sys, fvec pos, const std::string &contents)
    : entity(sys, team::INTERACTIVE), m_pos(pos), m_which(-1), m_state(-1)
{
    m_bbox = irect::centered(24, 24).offset(m_pos);
    if (contents.empty()) {
        std::printf("WARNING: No treasure contents at (%f, %f)\n",
                    pos.x, pos.y);
    } else {
        if (contents.size() != 2)
            core::die("Invalid treasure contents (invalid length)");
        m_which = contents[0] - '1';
        m_state = contents[1] - '1' + 1;
        if (m_which < 0 || m_which >= 3 || m_state < 1 || m_state >= 5)
            core::die("Invalid treasure contents");
    }
}

chest::~chest()
{ }

void chest::interact()
{
    if (m_which < 0)
        return;
    m_system.audio().play_music(
        m_state == 4 ? "fanfare_2" : "fanfare_1", true);
    auto &s = m_system.state();
    s.treasure[m_which] = m_state;
    m_system.add_entity(new signal_glyph(
        m_system, fvec(m_pos), ::graphics::treasure_sprite(m_which, m_state),
        "main_wake", false));
    m_team = team::DEAD;
}

void chest::draw(::graphics::system &gr, int reltime)
{
    (void)reltime;
    gr.add_sprite(
        sprite::CHEST,
        m_pos,
        orientation::NORMAL);
    if (m_system.test_hover(m_bbox)) {
        gr.add_sprite(
            ui::ARROW,
            m_pos + ivec(0, 24),
            orientation::NORMAL);
    }
}

// ======================================================================

enemy::enemy(entity_system &sys, fvec pos,
             ::graphics::anysprite actor,
             ::graphics::anysprite shot1, ::graphics::anysprite shot2)
    : entity(sys, team::FOE),
      physics(irect::centered(8, 20), pos, fvec::zero()),
      m_actor(actor),
      m_shot1(shot1),
      m_shot2(shot2),
      m_health(sys.state().enemy_health)
{ }

enemy::~enemy()
{ }

void enemy::update()
{
    m_enemy.update(m_system, *this, stats::prof);
    if (m_enemy.start_attack()) {
        m_system.audio().play_sfx(sfx::ENEMY_SHOOT);
        m_system.spawn_shot(
            team::FOE_SHOT, physics.pos, fvec(m_enemy.m_targetpos),
            stats::prof.shotspeed, m_shot1, m_shot2, SHOT_DELAY);
    }

    walking.update(physics, stats::player_walk);
    physics.update(m_system, *this);
}

void enemy::damage(int amount)
{
    m_health -= amount;
    if (m_health <= 0) {
        m_team = team::DEAD;
        m_system.add_entity(new poof(m_system, physics.pos));
        m_system.audio().play_sfx(sfx::ENEMY_DIE);
    } else {
        m_system.audio().play_sfx(sfx::ENEMY_HIT);
    }
}

void enemy::draw(::graphics::system &gr, int reltime)
{
    gr.add_sprite(
        m_actor,
        physics.get_pos(reltime),
        orientation::NORMAL);
}

// ======================================================================

shot::shot(entity_system &sys, team t, fvec pos, fvec vel, int time,
         ::graphics::anysprite sp1, ::graphics::anysprite sp2)
    : entity(sys, t),
      projectile(irect::centered(10, 10), pos, vel, 1),
      time(time),
      m_sp1(sp1), m_sp2(sp2)
{ }

shot::~shot()
{ }

void shot::update()
{
    time--;
    if (time <= 0)
        projectile.update(m_system, *this);
    if (time < -1000)
        m_team = team::DEAD;
}

void shot::draw(::graphics::system &gr, int reltime)
{
    gr.add_sprite(
        time > 0 ? m_sp1 : m_sp2,
        projectile.get_pos(reltime),
        orientation::NORMAL);
}

// ======================================================================

static const int POOF_FRAMETIME = 3;

poof::poof(entity_system &sys, fvec pos)
    : entity(sys, team::AMBIENT), m_pos(pos), m_time(0)
{ }

poof::~poof()
{ }

void poof::update()
{
    m_time++;
    if (m_time > POOF_FRAMETIME * 3)
        m_team = team::DEAD;
}

void poof::draw(::graphics::system &gr, int reltime)
{
    (void)reltime;
    anysprite s;
    switch (m_time / POOF_FRAMETIME) {
    case 0: s = sprite::POOF1; break;
    case 1: s = sprite::POOF2; break;
    case 2: s = sprite::POOF3; break;
    default: return;
    }
    gr.add_sprite(s, m_pos, orientation::NORMAL);
}

// ======================================================================

glyph::glyph(entity_system &sys, fvec pos, ::graphics::anysprite sp)
    : entity(sys, team::AMBIENT), m_pos(pos), m_sprite(sp)
{ }

glyph::~glyph()
{ }

void glyph::draw(::graphics::system &gr, int reltime)
{
    (void)reltime;
    gr.add_sprite(m_sprite, m_pos, orientation::NORMAL);
}

// ======================================================================

static const int SIGNAL_RISETIME = 16;
static const int SIGNAL_HOVERTIME = 32;
static const int SIGNAL_RISEDISTANCE = 24;

signal_glyph::signal_glyph(entity_system &sys, fvec pos,
                           ::graphics::anysprite sp,
                           const std::string &target, bool is_player_death)
    : entity(sys, team::AMBIENT),
      m_sprite(sp), m_pos(pos), m_target(target), m_time(0),
      m_is_player_death(is_player_death)
{ }

signal_glyph::~signal_glyph()
{ }

void signal_glyph::update()
{
    m_time++;
    if (m_time == SIGNAL_RISETIME + SIGNAL_HOVERTIME) {
        m_time = -1;
        m_team = team::DEAD;
        if (!m_target.empty() &&
            (!m_system.is_player_dead || m_is_player_death))
            m_system.nextlevel = std::move(m_target);
    }
}

void signal_glyph::draw(::graphics::system &gr, int reltime)
{
    float delta;
    if (m_time < SIGNAL_RISETIME)
        delta = (m_time * defs::FRAMETIME + reltime) *
            ((float) SIGNAL_RISEDISTANCE /
                (SIGNAL_RISETIME * defs::FRAMETIME));
    else
        delta = SIGNAL_RISEDISTANCE;
    gr.add_sprite(
        m_sprite,
        m_pos + ivec(0, std::floor(delta)),
        orientation::NORMAL);
}

}
