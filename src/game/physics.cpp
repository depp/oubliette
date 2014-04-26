/* Copyright 2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "../defs.hpp"
#include "defs.hpp"
#include "physics.hpp"
namespace game {

void physics_system::update()
{
    const scalar dt = 1e-3 * defs::FRAMETIME;
    for (auto i = objects.begin(), e = objects.end(); i != e; i++) {
        auto obj = *i;
        i->lastpos = i->pos;
        vec2 accel = obj.accel;
        vec2 pos = obj.pos + dt * obj.vel + (dt * dt / 2) * accel;
        vec2 vel = obj.vel + dt * accel;

        i->pos = pos;
        i->vel = vel;
    }
}

physics_object &physics_system::create(int object)
{
    if (object < 0)
        core::die("negative object");
    if ((std::size_t)object >= objects.size()) {
        int oldsize = objects.size();
        objects.resize(object + 1);
        for (int i = oldsize; i < object; i++)
            objects[i].active = false;
    }
    physics_object &data = objects[object];
    if (data.active)
        core::die("object exists");
    data.active = true;
}

void physics_system::destroy(int object)
{
    if (object < 0 || (std::size_t)object > objects.size())
        return;
    objects[object].active = false;
}

}
