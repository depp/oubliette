/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "vec.hpp"

bool frect::test_intersect(const frect &a, const frect &b)
{
    return a.x0 <= b.x1 && a.x1 >= b.x0 &&
        a.y0 <= b.y1 && a.y1 >= b.y0;
}

fvec frect::center() const
{
    return fvec(0.5f * (x0 + x1), 0.5f * (y0 + y1));
}

frect frect::offset(fvec v) const
{
    return frect(x0 + v.x, y0 + v.y, x1 + v.x, y1 + v.y);
}

fvec frect::nearest(fvec v) const
{
    fvec r;

    if (v.x < x0)
        r.x = x0;
    else if (v.x > x1)
        r.x = x1;
    else
        r.x = v.x;

    if (v.y < y0)
        r.y = y0;
    else if (v.y > y1)
        r.y = y1;
    else
        r.y = v.y;

    return r;
}

irect irect::offset(ivec v) const
{
    return irect(x0 + v.x, y0 + v.y, x1 + v.x, y1 + v.y);
}

irect irect::expand(int amt) const
{
    return irect(x0 - amt, y0 - amt, x1 + amt, y1 + amt);
}

irect irect::expand(const irect &r) const
{
    return irect(x0 + r.x0, y0 + r.y0, x1 + r.x1, y1 + r.y1);
}

bool irect::contains(ivec v) const
{
    return x0 <= v.x && v.x < x1 && y0 <= v.y && v.y < y1;
}

bool irect::test_intersect(const irect &a, const irect &b)
{
    return a.x0 < b.x1 && a.x1 > b.x0 && a.y0 < b.y1 && a.y1 > b.y0;
}

ivec irect::center() const
{
    return ivec(x0 + (x1 - x0) / 2, y0 + (y1 - y0) / 2);
}

irect irect::centered(int w, int h)
{
    return irect(-(w/2), -(h/2), w - (w/2), h - (h/2));
}
