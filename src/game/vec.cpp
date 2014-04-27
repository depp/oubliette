/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "vec.hpp"
namespace game {

bool rect::test_intersect(const rect &a, const rect &b)
{
    return a.min.x <= b.max.x && a.max.x >= b.min.x &&
        a.min.y <= b.max.y && a.max.y >= b.min.y;
}

rect rect::offset(vec2 v) const
{
    return rect(min + v, max + v);
}

vec2 rect::nearest(vec2 v) const
{
    vec2 r;

    if (v.x < min.x)
        r.x = min.x;
    else if (v.x > max.x)
        r.x = max.x;
    else
        r.x = v.x;

    if (v.y < min.y)
        r.y = min.y;
    else if (v.y > max.y)
        r.y = max.y;
    else
        r.y = v.y;

    return r;
}

}
