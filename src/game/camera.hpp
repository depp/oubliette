/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_CAMERA_HPP
#define LD_GAME_CAMERA_HPP
#include "vec.hpp"
namespace game {

/// The camera system.
class camera_system {
private:
    /// Whether there are bounds.
    bool has_bounds_;
    /// Whether the target has been set.
    bool targetset_;
    /// The limits of the camera focus.
    rect bounds_;
    /// The target rectangle.
    rect target_;
    /// The most recent camera position.
    vec2 lastpos_;
    /// The current camera position.
    vec2 pos_;
    /// The point that the camera is moving towards.
    vec2 targetpt_;
    /// The maximum camera speed.
    float maxspeed_;

public:
    camera_system();
    camera_system(const rect &bounds);

    /// Set the camera target.
    void set_target(const rect &target);
    /// Update the camera system.
    void update();
    /// Get the camera position.
    vec2 get_pos(int reltime) const;
};

}
#endif
