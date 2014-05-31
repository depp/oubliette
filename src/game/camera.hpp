/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_CAMERA_HPP
#define LD_GAME_CAMERA_HPP
#include "base/vec.hpp"
namespace game {

/// The camera system.
class camera_system {
private:
    /// Whether there are bounds.
    bool has_bounds_;
    /// Whether the target has been set.
    bool targetset_;
    /// The limits of the camera focus.
    frect bounds_;
    /// The target rectangle.
    frect target_;
    /// The most recent camera position.
    fvec lastpos_;
    /// The current camera position.
    fvec pos_;
    /// The point that the camera is moving towards.
    fvec targetpt_;
    /// The maximum camera speed.
    float maxspeed_;

public:
    camera_system();
    camera_system(const frect &bounds);

    /// Set the camera target.
    void set_target(const frect &target);
    /// Update the camera system.
    void update();
    /// Get the camera position.
    fvec get_pos(int reltime) const;
};

}
#endif
