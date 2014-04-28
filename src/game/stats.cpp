/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "stats.hpp"
namespace game {

const float stats::gravity = 600.0f;

const walking_stats stats::player_walk = {
    1200.0f, 120.0f, 300.0f, 150.0f
};

const jumping_stats stats::player_jump = {
    25, 400.0f, 180.0f, true
};

const struct enemy_stats stats::prof = {
    15, 15, 35, 140, 75
};

}
