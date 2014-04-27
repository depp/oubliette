/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_CONTROL_HPP
#define LD_GAME_CONTROL_HPP
#include "key.hpp"
namespace game {

/// Control (i.e., player input) subsystem.
class control_system {
private:
    const static int NKEYS = 9;

    int keys[NKEYS];

    /// Read two button states and convert them to a continuous value.
    float button_to_axis(key negative, key positive) const;

public:
    control_system();

    /// Clear the state of all keys.
    void clear();
    /// Update the state for the next frame.
    void update();
    /// Get the state for a key.
    bool get_key(key k) const;
    /// Get whether the key was pressed this frame.
    bool get_key_instant(key k) const;
    /// Set the state for a key.
    void set_key(key k, bool state);
    /// Get the x-axis state, in the range [-1,1].
    float get_xaxis() const;
    /// Get the y-axis state, in the range [-1,1].
    float get_yaxis() const;
};

}
#endif
