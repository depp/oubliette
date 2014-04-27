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

physics_component::physics_component(rect bbox, vec2 pos, vec2 vel)
    : bbox(bbox), pos(pos), vel(vel)
{ }

void physics_component::update(state &st, entity &e)
{
    const scalar dt = 1e-3 * defs::FRAMETIME;

    lastpos = pos;
    vec2 frame_accel = accel;
    vec2 new_pos = pos + dt * vel + (dt * dt / 2) * frame_accel;
    vec2 new_vel = vel + dt * frame_accel;

    rect new_bbox = bbox.offset(new_pos);
    if (st.level().hit_test(new_bbox)) {
        new_pos = pos;
        new_vel = vec2::zero();
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
    : gravity(vec2::zero()), xmove(0.0f), ymove(0.0f)
{ }

void walking_component::update(state &st, physics_component &physics)
{
    (void)&st;
    (void)&physics;
    physics.accel = gravity;
    xmove = 0.0f;
    ymove = 0.0f;
}

// ======================================================================

player::player(state &st, vec2 pos)
    : entity(st, team::FRIEND),
      physics(rect(-4, -10, 4, 10), pos, vec2::zero())
{
    walking.gravity = vec2(0, -100);
}

player::~player()
{ }

void player::update()
{
    walking.update(m_state, physics);
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
