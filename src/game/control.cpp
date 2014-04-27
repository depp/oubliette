/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "control.hpp"
#include "../defs.hpp"
namespace game {

control_system::control_system()
{
    clear();
}

float control_system::button_to_axis(key negative, key positive) const
{
    bool nstate = keys[static_cast<int>(negative)];
    bool pstate = keys[static_cast<int>(positive)];
    return 0.0f + (nstate ? -1.0f : 0.0f) + (pstate ? +1.0f : 0.0f);
}

void control_system::clear()
{
    for (int i = 0; i < NKEYS; i++)
        keys[i] = 0;
}

void control_system::update()
{
    for (int i = 0; i < NKEYS; i++) {
        if (keys[i] == 1)
            keys[i] = 2;
    }
}

bool control_system::get_key(key k) const
{
    int index = static_cast<int>(k);
    if (index < 0 || index >= NKEYS)
        core::die("Unknown key");
    return keys[index] != 0;
}

bool control_system::get_key_instant(key k) const
{
    int index = static_cast<int>(k);
    if (index < 0 || index >= NKEYS)
        core::die("Unknown key");
    return keys[index] == 1;
}

void control_system::set_key(key k, bool state)
{
    int index = static_cast<int>(k);
    if (index < 0 || index >= NKEYS)
        core::die("Unknown key");
    if (state) {
        if (keys[index] == 0)
            keys[index] = 1;
    } else {
        keys[index] = 0;
    }
}

float control_system::get_xaxis() const
{
    return button_to_axis(key::LEFT, key::RIGHT);
}

float control_system::get_yaxis() const
{
    return button_to_axis(key::DOWN, key::UP);
}

}
