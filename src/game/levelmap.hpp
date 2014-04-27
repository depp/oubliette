/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_LEVELMAP_HPP
#define LD_GAME_LEVELMAP_HPP
#include <string>
#include "../image.hpp"
#include "vec.hpp"
namespace game {

/// Level collision map.
class levelmap {
private:
    image::bitmap map;

public:
    /// Do a hit test against a rectangle.
    bool hit_test(irect r) const;
    /// Returns the number of pixels to move up to clear collisions.
    int hit_y0(irect r) const;
    /// Returns the number of pixels to move down to clear collisions.
    int hit_y1(irect r) const;
    /// Load the collision map for a level.
    void set_level(const std::string &name);
};

}
#endif
