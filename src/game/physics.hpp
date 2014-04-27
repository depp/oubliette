/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_PHYSICS_HPP
#define LD_GAME_PHYSICS_HPP
#include <vector>
#include "vec.hpp"
namespace game {
class levelmap;

struct physics_object {
    bool active;
    vec2 extent_min, extent_max;
    vec2 lastpos;
    vec2 pos;
    vec2 vel;
    vec2 accel;
};

class physics_system {
    const levelmap &level_;

public:
    physics_system(const levelmap &level);

    std::vector<physics_object> objects;

    void update();
    physics_object &create(int object);
    void destroy(int object);
};

}
#endif
