/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_DEFS_HPP
#define LD_GAME_DEFS_HPP
#include "vec.hpp"
namespace game {

struct defs {
    /// Number of milliseconds between updates.
    static const int FRAMETIME = 32; // 31.25 Hz

    /// Maximum interval between updates.
    static const int MAXUPDATE = 500;

    static vec2 interp(vec2 a, vec2 b, int reltime)
    {
        scalar frac = (scalar) reltime * (scalar)(1.0 / defs::FRAMETIME);
        return a + (b - a) * frac;
    }
};

}
#endif
