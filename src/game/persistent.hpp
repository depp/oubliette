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

    persistent_state()
        : health(0), maxhealth(0), treasure{0, 0, 0}, hittime(0)
    { }
};

}
#endif
