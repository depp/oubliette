/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "stats.hpp"
namespace game {

const walking_stats stats::player = {
    600.0f,
    // Walking
    1200.0f, 120.0f, 300.0f, 150.0f, 3,
    // Jumping
    25, 400.0f, 180.0f, true
};

}
