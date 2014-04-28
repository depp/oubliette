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
#include <cstdio>
#include <algorithm>
namespace game {

using ::graphics::sprite;
using ::sprite::orientation;

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

entity_system::entity_system(const control_system &control,
                             const std::string &levelname,
                             const std::string &lastlevel)
    : control_(control), levelname_(levelname)
{
    level_.set_level(levelname);
    auto data = leveldata::read_level(levelname);
    auto b = data.begin(), e = data.end();
    spawnpoint *pspawn = nullptr, *dspawn = nullptr, *dspawn2 = nullptr;

    for (auto i = b; i != e; i++) {
        vec2 pos(i->x, i->y);
        switch (i->type) {
        case spawntype::PLAYER:
            pspawn = &*i;
            break;

        case spawntype::DOOR:
            entities_.emplace_back(new door(*this, pos, i->data));
            if (i->data == lastlevel)
                dspawn = &*i;
            dspawn2 = &*i;
            break;

        case spawntype::CHEST:
            entities_.emplace_back(new chest(*this, pos, i->data));
            break;

        case spawntype::WOMAN:
            entities_.emplace_back(new woman(*this, pos));
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
}

void entity_system::draw(::graphics::system &gr, int reltime)
{
    gr.set_camera_pos(camera_.get_pos(reltime));
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

physics_component::physics_component(irect bbox, vec2 pos, vec2 vel)
    : bbox(bbox), pos(pos), vel(vel), on_floor(false)
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
}

vec2 physics_component::get_pos(int reltime)
{
    return defs::interp(lastpos, pos, reltime);
}

// ======================================================================

walking_component::walking_component()
    : xmove(0.0f), ymove(0.0f),
      jumptime(0), jstate(jumpstate::READY)
{ }

void walking_component::update(entity_system &sys, physics_component &physics,
                               const walking_stats &stats)
{
    (void)&sys;
    vec2 accel = vec2(0, -stats.gravity);

    bool on_floor = physics.on_floor;
    // std::printf("on floor: %s\n", on_floor ? "true" : "false");

    float max_xspeed = on_floor ? stats.xspeed_ground : stats.xspeed_air;
    float max_xaccel = on_floor ? stats.xaccel_ground : stats.xaccel_air;
    float xaccel = (max_xspeed * xmove - physics.vel.x) * INV_DT;
    if (xaccel > max_xaccel)
        xaccel = max_xaccel;
    else if (xaccel < -max_xaccel)
        xaccel = -max_xaccel;
    accel.x += xaccel;

    if (on_floor) {
        jumptime = 0;
        if (ymove > 0.5f) {
            if (jstate == jumpstate::READY) {
                jumptime = stats.jumptime;
                jstate = jumpstate::JUMP1;
                if (stats.jumpspeed > physics.vel.y)
                    accel.y += (stats.jumpspeed - physics.vel.y) * INV_DT;
            }
        } else {
            jstate = jumpstate::READY;
        }
    } else {
        if (ymove >= 0.50f) {
            if (jumptime > 0) {
                jumptime--;
                accel.y += stats.jumpaccel * ymove;
            } else if (jstate == jumpstate::READY && stats.can_doublejump) {
                jumptime = stats.jumptime;
                jstate = jumpstate::JUMP2;
                if (stats.jumpspeed > physics.vel.y)
                    accel.y += (stats.jumpspeed - physics.vel.y) * INV_DT;
            }
        } else {
            jumptime = 0;
            if (jstate == jumpstate::JUMP1)
                jstate = jumpstate::READY;
        }
    }

    physics.accel = accel;
    xmove = 0.0f;
    ymove = 0.0f;
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
    walking.ymove = m_system.control().get_yaxis();
    walking.update(m_system, physics, stats::player);
    physics.update(m_system, *this);

    m_system.set_camera_target(CAMERA.offset(physics.pos));
    m_system.set_hover(ivec(physics.pos));

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
    : entity(sys, team::INTERACTIVE), m_pos(pos), m_target(target)
{
    m_bbox = irect::centered(24, 32).offset(pos);
}

door::~door()
{ }

void door::interact()
{
    m_system.nextlevel = m_target;
}

void door::draw(::graphics::system &gr, int reltime)
{
    (void)reltime;
    gr.add_sprite(
        sprite::DOOR2,
        m_pos + vec2(-12, -16),
        orientation::NORMAL);
    if (m_system.test_hover(m_bbox)) {
        gr.add_sprite(
            sprite::ARROW,
            m_pos + vec2(-8, +20),
            orientation::NORMAL);
    }
}

// ======================================================================

chest::chest(entity_system &sys, vec2 pos, const std::string contents)
    : entity(sys, team::INTERACTIVE), m_pos(pos), m_contents(contents)
{
    m_bbox = irect::centered(24, 24).offset(pos);
}

chest::~chest()
{ }

void chest::interact()
{ }

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

woman::woman(entity_system &sys, vec2 pos)
    : entity(sys, team::FOE),
      physics(irect::centered(8, 20), pos, vec2::zero())
{ }

woman::~woman()
{ }

void woman::update()
{
    walking.update(m_system, physics, stats::player);
    physics.update(m_system, *this);
}

void woman::draw(::graphics::system &gr, int reltime)
{
    gr.add_sprite(
        sprite::WOMAN,
        physics.get_pos(reltime) + vec2(-8, -12),
        orientation::NORMAL);
}

// ======================================================================

}
