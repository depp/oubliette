/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "entity.hpp"
#include "defs.hpp"
#include "state.hpp"
#include "graphics.hpp"
#include <cstdio>
namespace game {

using ::graphics::sprite;
using ::sprite::orientation;

static const scalar DT = 1e-3 * defs::FRAMETIME;
static const scalar INV_DT = 1.0 / (1e-3 * defs::FRAMETIME);

static const walking_stats PLAYER_STATS = {
    300.0f,
    // Walking
    600.0f, 120.0f, 80.0f, 150.0f, 3,
    // Jumping
    25, 200.0f, 150.0f, true
};

// ======================================================================

entity::entity(state &st, team t)
    : m_state(st), m_bbox(0, 0, 0, 0),
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

void physics_component::update(state &st, entity &e)
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
    const levelmap &level = st.level();
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
    scalar frac = (scalar) reltime * (scalar)(1.0 / defs::FRAMETIME);
    return lastpos + (pos - lastpos) * frac;
}

// ======================================================================

walking_component::walking_component()
    : gravity(vec2::zero()), xmove(0.0f), ymove(0.0f),
      jumptime(0), jstate(jumpstate::READY)
{ }

void walking_component::update(state &st, physics_component &physics,
                               const walking_stats &stats)
{
    (void)&st;
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
                // std::puts("JUMP");
                jumptime = stats.jumptime;
                jstate = jumpstate::JUMP1;
                accel.y += stats.jumpspeed * INV_DT;
            }
        } else {
            jstate = jumpstate::READY;
        }
    } else if (jumptime > 0) {
        jumptime--;
        if (ymove >= 0.25f) {
            accel.y += stats.jumpaccel * ymove;
        } else {
            jumptime = 0;
            if (jstate == jumpstate::JUMP1)
                jstate = jumpstate::READY;
        }
    } else {
        if (ymove > 0.5f) {
            if (jstate == jumpstate::READY && stats.can_doublejump) {
                // std::puts("DOUBLE JUMP");
                jumptime = stats.jumptime;
                jstate = jumpstate::JUMP2;
                accel.y += stats.jumpspeed * INV_DT;
            }
        } else {
            if (jstate == jumpstate::JUMP1)
                jstate = jumpstate::READY;
        }
    }

    physics.accel = accel;
    xmove = 0.0f;
    ymove = 0.0f;
}

// ======================================================================

player::player(state &st, vec2 pos)
    : entity(st, team::FRIEND),
      physics(irect::centered(8, 20), pos, vec2::zero())
{
    walking.gravity = vec2(0, -100);
    // walking.gravity = vec2::zero();
}

player::~player()
{ }

void player::update()
{
    walking.xmove = m_state.control().get_xaxis();
    walking.ymove = m_state.control().get_yaxis();
    walking.update(m_state, physics, PLAYER_STATS);
    physics.update(m_state, *this);
}

void player::damage(int amount)
{
    (void)amount;
}

void player::draw(::graphics::system &gr, int reltime)
{
    gr.add_sprite(
        sprite::PLAYER,
        physics.get_pos(reltime) + vec2(-8, -12),
        orientation::NORMAL);
}

// ======================================================================

}
