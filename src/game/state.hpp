/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_STATE_HPP
#define LD_GAME_STATE_HPP
#include "allocator.hpp"
#include "graphics.hpp"
#include "physics.hpp"
namespace game {

class state {
    unsigned frametime_;

public:
    allocator_system allocator;
    physics_system physics;
    graphics_system graphics;

    void init();
    void term();
    void draw(unsigned time);

    void event_click(int x, int y, int button);
};

}
#endif
