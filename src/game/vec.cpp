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

rect rect::offset(vec2 v)
{
    return rect(min + v, max + v);
}

}
