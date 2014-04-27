/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "state.hpp"
#include "../opengl.hpp"
#include <cstdio>
namespace game {

state::state()
    : initted_(false),
      physics(level)
{ }

void state::draw(unsigned time)
{
    unsigned delta;
    if (initted_) {
        delta = time - frametime_;
    } else {
        delta = 0;
        initted_ = true;
        frametime_ = time;
    }
    int nframes;
    if ((unsigned)delta > defs::MAXUPDATE) {
        std::puts("Lag!");
        nframes = 1;
        frametime_ = time;
    } else {
        nframes = delta / defs::FRAMETIME;
        frametime_ += nframes * defs::FRAMETIME;
    }
    for (int i = 0; i < nframes; i++) {
        physics.update();
    }

    graphics.draw(*this, time - frametime_);
}

void state::init()
{
    graphics.init();
    set_level("main_wake");
}

void state::term()
{
    graphics.term();
}

void state::event_click(int x, int y, int button)
{
    if (button != 1 && button != 3)
        return;
    auto obj = allocator.create();
    auto &p = physics.create(obj);
    p.extent_min = vec2(-6, -10);
    p.extent_max = vec2(6, 10);
    p.lastpos = p.pos = vec2(x, y);
    p.vel = vec2::zero();
    p.accel = vec2(0, -100);
}

void state::set_level(const std::string &name)
{
    level.set_level(name);
    graphics.set_level("main_wake");
}

}
