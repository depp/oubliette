/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "sprite.hpp"

namespace sprite {

/// Compose two orientations.
orientation operator*(orientation x, orientation y)
{
    int xi = static_cast<int>(x), yi = static_cast<int>(y);
    int xm = (xi >> 2) & 1, ym = (yi >> 2) & 1;
    int xr = (xi & 3), yr = (yi & 3);
    if (ym)
        xr *= 3;
    return static_cast<orientation>(((xm ^ ym) << 2) | ((xr + yr) & 3));
}

}
