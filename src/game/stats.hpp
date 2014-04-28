/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_STATS_HPP
#define LD_GAME_STATS_HPP
namespace game {

/// Walking statistics.
struct walking_stats {
    float gravity;
    float xaccel_ground;
    float xspeed_ground;
    float xaccel_air;
    float xspeed_air;
    int floordepth;
    int jumptime;
    float jumpaccel;
    float jumpspeed;
    bool can_doublejump;
};

/// Global object statistics.
struct stats {
    static const struct walking_stats player;
};

}
#endif
