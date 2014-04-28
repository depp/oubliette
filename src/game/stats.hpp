/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_STATS_HPP
#define LD_GAME_STATS_HPP
namespace game {

/// Walking statistics.
struct walking_stats {
    float accel_ground;
    float speed_ground;
    float accel_air;
    float speed_air;
};

// Jumping statistics.
struct jumping_stats {
    int jumptime;
    float accel;
    float speed;
    bool can_doublejump;
};

/// Enemy statistics.
struct enemy_stats {
    int reaction;
    int interval;
    int attacktime;
    int xsight;
    int ysight;
    float shotspeed;
};

/// Global object statistics.
struct stats {
    static const float gravity;
    static const struct walking_stats player_walk;
    static const struct jumping_stats player_jump;
    static const struct enemy_stats prof;
};

}
#endif
