/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "levelmap.hpp"
#include <cstdlib>
#include <cmath>
namespace game {

bool levelmap::hit_test(irect r) const
{
    const unsigned char *data = map.data;
    int w = map.width, h = map.height, rb = map.rowbytes;
    if (r.x0 < 0 || r.x1 >= w)
        return true;
    int y0 = r.y0 >= 0 ? r.y0 : 0;
    int y1 = r.y1 <= h ? r.y1 : h;
    for (int y = y0; y < y1; y++) {
        unsigned hit = 0;
        const unsigned char *row = data + rb * y;
        for (int x = r.x0; x <= r.x1; x++)
            hit |= row[x];
        if (hit != 0)
            return true;
    }
    return false;
}

int levelmap::hit_y0(irect r) const
{
    const unsigned char *data = map.data;
    int w = map.width, h = map.height, rb = map.rowbytes;
    if (r.x0 < 0 || r.x1 >= w)
        return r.y1 - r.y0;
    int y0 = r.y0 >= 0 ? r.y0 : 0;
    int y1 = r.y1 <= h ? r.y1 : h;
    for (int y = y1; y > y0; y--) {
        unsigned hit = 0;
        const unsigned char *row = data + rb * (y - 1);
        for (int x = r.x0; x <= r.x1; x++)
            hit |= row[x];
        if (hit != 0)
            return y - r.y0;
    }
    return 0;
}

int levelmap::hit_y1(irect r) const
{
    const unsigned char *data = map.data;
    int w = map.width, h = map.height, rb = map.rowbytes;
    if (r.x0 < 0 || r.x1 >= w)
        return r.y1 - r.y0;
    int y0 = r.y0 >= 0 ? r.y0 : 0;
    int y1 = r.y1 <= h ? r.y1 : h;
    for (int y = y0; y < y1; y++) {
        unsigned hit = 0;
        const unsigned char *row = data + rb * y;
        for (int x = r.x0; x <= r.x1; x++)
            hit |= row[x];
        if (hit != 0)
            return r.y1 - y;
    }
    return 0;
}

void levelmap::set_level(const std::string &name)
{
    std::string fullpath("level/");
    fullpath += name;
    fullpath += ".png";
    map = image::bitmap::load(fullpath);
}

}
