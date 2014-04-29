/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_PERSISTENT_HPP
#define LD_GAME_PERSISTENT_HPP
namespace game {

struct persistent_state {
    int health;
    int maxhealth;

    int treasure[3];

    int hittime;
    int enemy_health;

    persistent_state()
        : health(-1), maxhealth(-1), treasure{0, 0, 0}, hittime(0),
          enemy_health(1)
    { }
};

}
#endif
