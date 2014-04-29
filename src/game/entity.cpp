/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "entity.hpp"
#include "../defs.hpp"
#include "control.hpp"
#include "defs.hpp"
#include "graphics.hpp"
#include "leveldata.hpp"
#include "stats.hpp"
#include "persistent.hpp"
#include <cstdio>
#include <algorithm>
namespace game {

using ::graphics::sprite;
using ::sprite::orientation;

static const int HIT_TIME = 25;
static const int SHOT_DELAY = 8;

static const scalar DT = 1e-3 * defs::FRAMETIME;
static const scalar INV_DT = 1.0 / (1e-3 * defs::FRAMETIME);

static const int CAMERA_X = 16;
static const int CAMERA_Y = 16;
static const rect CAMERA(-CAMERA_X, -CAMERA_Y, +CAMERA_X, +CAMERA_Y);

struct entity_is_dead {
    bool operator()(const std::unique_ptr<entity> &p) {
        return p->m_team != team::DEAD;
    }
};

// ======================================================================

std::string door_name(const std::string &data)
{
    auto pos = data.rfind(':');
    if (pos == std::string::npos)
        return data;
    return data.substr(pos + 1);
}

entity_system::entity_system(persistent_state &state,
                             const control_system &control,
                             const std::string &levelname,
                             const std::string &lastlevel)
    : state_(state), control_(control), levelname_(levelname),
      is_click_(false)
{
    level_.set_level(levelname);
    auto data = leveldata::read_level(levelname);
    auto b = data.begin(), e = data.end();
    spawnpoint *pspawn = nullptr, *dspawn = nullptr, *dspawn2 = nullptr;
    std::string dname;

    for (auto i = b; i != e; i++) {
        vec2 pos(i->x, i->y);
        switch (i->type) {
        case spawntype::PLAYER:
            pspawn = &*i;
            break;

        case spawntype::DOOR:
            entities_.emplace_back(new door(*this, pos, i->data));
            if (door_name(i->data) == lastlevel)
                dspawn = &*i;
            dspawn2 = &*i;
            break;

        case spawntype::CHEST:
            entities_.emplace_back(new chest(*this, pos, i->data));
            break;

        case spawntype::PROF:
            entities_.emplace_back(
                new enemy(*this, pos,
                          sprite::PROFESSOR, sprite::BOOK1, sprite::BOOK2));
            break;

        case spawntype::WOMAN:
            entities_.emplace_back(
                new enemy(*this, pos,
                          sprite::WOMAN, sprite::MOUTH1, sprite::MOUTH2));
            break;

        case spawntype::PRIEST:
            entities_.emplace_back(
                new enemy(*this, pos,
                          sprite::PRIEST, sprite::SKULL, sprite::SKULL));
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
            new player(*this, vec2(pspawn->x, pspawn->y)));

    camera_ = camera_system(
        rect(vec2::zero(), vec2(level_.width(), level_.height())));
}

void entity_system::update()
{
    if (state_.hittime > 0)
        state_.hittime--;
    auto part = std::stable_partition(
        entities_.begin(), entities_.end(), entity_is_dead());
    entities_.erase(part, entities_.end());
    entities_.insert(
        entities_.end(),
        std::make_move_iterator(new_entities_.begin()),
        std::make_move_iterator(new_entities_.end()));
    new_entities_.clear();
    hover_trigger_ = ivec(-1000, -1000);
    for (auto i = entities_.begin(), e = entities_.end(); i != e; i++) {
        entity &ent = **i;
        ent.update();
    }
    camera_.update();
    is_click_ = false;
}

void entity_system::draw(::graphics::system &gr, int reltime)
{
    static const int BASE = 0, HURT = 6;
    float color[4];
    if (state_.hittime > 0) {
        graphics::blend(
            color, BASE, 0.5f, HURT, 1.0f,
            state_.hittime * (1.0f / HIT_TIME));
    } else {
        graphics::blend(color, BASE, 0.5f);
    }
    gr.set_blend_color(color);

    for (int i = 0; i < state_.maxhealth; i++) {
        gr.add_sprite(
            i < state_.health ? sprite::HEART1 : sprite::HEART2,
            vec2(core::PWIDTH - 16 - 16*i, core::PHEIGHT - 16),
            orientation::NORMAL,
            true);
    }

    for (int i = 0; i < 3; i++) {
        gr.add_sprite(
            ::graphics::treasure_sprite(i, state_.treasure[i]),
            vec2(2 + 18*i, core::PHEIGHT - 17),
            orientation::NORMAL,
            true);
    }

    vec2 camera = camera_.get_pos(reltime);
    lastcamera_ = ivec(camera);
    gr.set_camera_pos(camera);
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

void entity_system::set_camera_target(const rect &target)
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
        if (ent.m_team == t && ent.m_bbox.test_intersect(range))
            return &ent;
    }
    return nullptr;
}

void entity_system::die()
{
    state().hittime = HIT_TIME * 8;
    nextlevel = "!dead";
}

void entity_system::mouse_click(int x, int y, int button)
{
    if (button != 1 && button != 3)
        return;
    is_click_ = true;
    click_pos_ = ivec(x + lastcamera_.x - core::PWIDTH/2,
                      y + lastcamera_.y - core::PHEIGHT/2);
}

void entity_system::spawn_shot(
    team t, vec2 origin, vec2 target, float speed,
    ::graphics::sprite sp1, ::graphics::sprite sp2,
    int delay)
{
    origin += vec2(
        std::copysign(10.0f, target.x - origin.x),
        4.0f);
    vec2 delta = target - origin;
    float mag2 = delta.mag2();
    vec2 shotvel;
    if (mag2 < 16)
        shotvel = vec2::zero();
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

vec2 entity::center() const
{
    return vec2(0.5f * (float)(m_bbox.x0 + m_bbox.x1),
                0.5f * (float)(m_bbox.y0 + m_bbox.y1));
}

// ======================================================================

physics_component::physics_component(irect bbox, vec2 pos, vec2 vel)
    : bbox(bbox), pos(pos), vel(vel), accel(0, -stats::gravity),
      on_floor(false)
{ }

void physics_component::update(entity_system &sys, entity &e)
{
    static const int MAXSTEP = 3;

    on_floor = false;
    lastpos = pos;
    vec2 old_pos = pos;
    vec2 new_pos = old_pos + DT * vel + (DT * DT / 2) * accel;
    vec2 old_vel = vel;
    vec2 new_vel = old_vel + DT * accel;

    int x1 = (int)std::floor(new_pos.x);
    int y1 = (int)std::floor(new_pos.y);
    irect new_bbox = bbox.offset(x1, y1);
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
    accel = vec2(0, -stats::gravity);
}

vec2 physics_component::get_pos(int reltime)
{
    return defs::interp(lastpos, pos, reltime);
}

// ======================================================================

projectile_component::projectile_component(
    irect bbox, vec2 pos, vec2 vel, int damage)
    : bbox(bbox), lastpos(pos), pos(pos), vel(vel),
      is_hit(false), damage(damage)
{ }

void projectile_component::update(entity_system &sys, entity &e)
{
    if (is_hit)
        return;
    lastpos = pos;
    pos += vel * DT;

    int x1 = (int)std::floor(pos.x);
    int y1 = (int)std::floor(pos.y);
    e.m_bbox = bbox.offset(x1, y1);

    if (sys.level().hit_test(e.m_bbox))
        hit(sys, e);

    team enemy;
    switch (e.m_team) {
    case team::FRIEND_SHOT: enemy = team::FOE; break;
    case team::FOE_SHOT: enemy = team::FRIEND; break;
    default: return;
    }

    auto be = sys.entities().begin(), ee = sys.entities().end();
    for (auto i = be; i != ee; i++) {
        entity &target = **i;
        if (target.m_team != enemy)
            continue;
        if (!target.m_bbox.test_intersect(e.m_bbox))
            continue;
        target.damage(damage);
        hit(sys, e);
    }
}

void projectile_component::hit(entity_system &sys, entity &e)
{
    if (is_hit)
        return;
    is_hit = true;
    e.m_team = team::DEAD;
    sys.add_entity(new poof(sys, pos));
}

vec2 projectile_component::get_pos(int reltime)
{
    return defs::interp(lastpos, pos, reltime);
}

// ======================================================================

walking_component::walking_component()
    : xmove(0.0f)
{ }

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
}

// ======================================================================

#if 0
static void jump_simple(physics_component &physics, vec2 vel)
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
            m_targetpos = target->center();
        }
        break;

    case state::ALERT:
        m_time--;
        if (m_time == 0) {
            target = scan(sys, e, stats);
            if (target != nullptr) {
                m_state = state::ATTACK;
                m_time = 0;
                m_targetpos = target->center();
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

player::player(entity_system &sys, vec2 pos)
    : entity(sys, team::FRIEND),
      physics(irect::centered(8, 20), pos, vec2::zero())
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

    m_system.set_camera_target(CAMERA.offset(physics.pos));
    m_system.set_hover(ivec(physics.pos));

    if (m_system.is_click()) {
        auto target = m_system.click_pos();
        m_system.spawn_shot(
            team::FRIEND_SHOT,
            physics.pos,
            vec2(target.x, target.y),
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
        m_system.die();
}

void player::damage(int amt)
{
    auto &s = m_system.state();
    s.hittime += HIT_TIME * amt;
    if (s.maxhealth > 0) {
        s.health -= amt;
        if (s.health <= 0)
            m_system.die();
    }
}

void player::draw(::graphics::system &gr, int reltime)
{
    gr.add_sprite(
        sprite::PLAYER,
        physics.get_pos(reltime) + vec2(-8, -12),
        orientation::NORMAL);
}

// ======================================================================

door::door(entity_system &sys, vec2 pos, const std::string target)
    : entity(sys, team::INTERACTIVE), m_pos(pos), m_target(target),
      m_is_locked(false)
{
    m_bbox = irect::centered(24, 32).offset(pos);
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
    if (!m_is_locked)
        m_system.nextlevel = m_target;
}

void door::draw(::graphics::system &gr, int reltime)
{
    (void)reltime;
    gr.add_sprite(
        m_is_locked ? sprite::DOOR3 : sprite::DOOR2,
        m_pos + vec2(-12, -16),
        orientation::NORMAL);
    if (!m_is_locked && m_system.test_hover(m_bbox)) {
        gr.add_sprite(
            sprite::ARROW,
            m_pos + vec2(-8, +20),
            orientation::NORMAL);
    }
}

// ======================================================================

chest::chest(entity_system &sys, vec2 pos, const std::string &contents)
    : entity(sys, team::INTERACTIVE), m_pos(pos), m_which(-1), m_state(-1)
{
    m_bbox = irect::centered(24, 24).offset(pos);
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
    auto &s = m_system.state();
    s.treasure[m_which] = m_state;
}

void chest::draw(::graphics::system &gr, int reltime)
{
    (void)reltime;
    gr.add_sprite(
        sprite::CHEST,
        m_pos + vec2(-12, -12),
        orientation::NORMAL);
    if (m_system.test_hover(m_bbox)) {
        gr.add_sprite(
            sprite::ARROW,
            m_pos + vec2(-8, +16),
            orientation::NORMAL);
    }
}

// ======================================================================

enemy::enemy(entity_system &sys, vec2 pos,
             ::graphics::sprite actor,
             ::graphics::sprite shot1, ::graphics::sprite shot2)
    : entity(sys, team::FOE),
      physics(irect::centered(8, 20), pos, vec2::zero()),
      m_actor(actor),
      m_shot1(shot1),
      m_shot2(shot2)
{ }

enemy::~enemy()
{ }

void enemy::update()
{
    m_enemy.update(m_system, *this, stats::prof);
    if (m_enemy.start_attack()) {
        m_system.spawn_shot(
            team::FOE_SHOT, physics.pos, m_enemy.m_targetpos,
            stats::prof.shotspeed, m_shot1, m_shot2, SHOT_DELAY);
    }

    walking.update(physics, stats::player_walk);
    physics.update(m_system, *this);
}

void enemy::draw(::graphics::system &gr, int reltime)
{
    gr.add_sprite(
        m_actor,
        physics.get_pos(reltime) + vec2(-8, -12),
        orientation::NORMAL);
}

// ======================================================================

shot::shot(entity_system &sys, team t, vec2 pos, vec2 vel, int time,
         ::graphics::sprite sp1, ::graphics::sprite sp2)
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
        projectile.get_pos(reltime) + vec2(-8, -8),
        orientation::NORMAL);
}

// ======================================================================

static const int POOF_FRAMETIME = 3;

poof::poof(entity_system &sys, vec2 pos)
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
    sprite s;
    switch (m_time / POOF_FRAMETIME) {
    case 0: s = sprite::POOF1; break;
    case 1: s = sprite::POOF2; break;
    case 2: s = sprite::POOF3; break;
    default: return;
    }
    gr.add_sprite(s, m_pos + vec2(-8, -8), orientation::NORMAL);

}

// ======================================================================

}
