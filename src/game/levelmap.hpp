/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_LEVELMAP_HPP
#define LD_GAME_LEVELMAP_HPP
#include <string>
#include "../image.hpp"
namespace game {

class levelmap {
private:
    image::bitmap map;

public:
    /// Do a hit test against a rectangle.
    /// Note that xmin == xmax means 1 pixel wide.
    bool hit_test(int xmin, int ymin, int xmax, int ymax) const;

    void set_level(const std::string &name);
};

}
#endif
