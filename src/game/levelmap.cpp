/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "levelmap.hpp"
#include <cstdlib>
namespace game {

bool levelmap::hit_test(int xmin, int ymin, int xmax, int ymax) const
{
    if (xmin > xmax || ymin > ymax)
        return false;
    const unsigned char *data = map.data;
    int w = map.width, h = map.height, rb = map.rowbytes;
    if (xmin < 0 || xmax >= w)
        return true;
    if (ymax < 0 || ymin >= h)
        return false;
    if (ymin < 0)
        ymin = 0;
    if (ymax >= h)
        ymax = h - 1;
    unsigned hit = 0;
    for (int y = ymin; y <= ymax; y++) {
        const unsigned char *row = data + rb * y;
        for (int x = xmin; x <= xmax; x++)
            hit |= row[x];
    }
    return hit != 0;
}

void levelmap::set_level(const std::string &name)
{
    std::string fullpath("level/");
    fullpath += name;
    fullpath += ".png";
    map = image::bitmap::load(fullpath);
}

}
