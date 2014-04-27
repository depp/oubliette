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
        keys[i] = false;
}

bool control_system::get_key(key k) const
{
    int index = static_cast<int>(k);
    if (index < 0 || index >= NKEYS)
        core::die("Unknown key");
    return keys[index];
}

void control_system::set_key(key k, bool state)
{
    int index = static_cast<int>(k);
    if (index < 0 || index >= NKEYS)
        core::die("Unknown key");
    keys[index] = state;
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
