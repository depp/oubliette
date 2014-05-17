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

    static fvec interp(fvec a, fvec b, int reltime)
    {
        float frac = reltime * (1.0f / defs::FRAMETIME);
        return a + (b - a) * frac;
    }
};

}
#endif
