/* Copyright 2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_DEFS_HPP
#define LD_GAME_DEFS_HPP

namespace game {

struct defs {
    /// Number of milliseconds between updates.
    static const int FRAMETIME = 32; // 31.25 Hz

    /// Maximum interval between updates.
    static const int MAXUPDATE = 500;
};

}

#endif
