/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_STATE_HPP
#define LD_GAME_STATE_HPP
#include <string>
#include "allocator.hpp"
#include "graphics.hpp"
#include "levelmap.hpp"
#include "physics.hpp"
namespace game {

class state {
private:
public:
    bool initted_;
    unsigned frametime_;

    allocator_system allocator;
    levelmap level;
    graphics_system graphics;
    physics_system physics;

    state();

    void init();
    void term();
    void draw(unsigned time);

    void event_click(int x, int y, int button);

    void set_level(const std::string &name);
};

}
#endif
