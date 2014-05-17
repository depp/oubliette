/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "camera.hpp"
#include "defs.hpp"
#include "../defs.hpp"
#include <cmath>
namespace game {

camera_system::camera_system()
    : has_bounds_(false), targetset_(false),
      maxspeed_(120.0f)
{
    lastpos_ = pos_ = targetpt_ = fvec::zero();
}

camera_system::camera_system(const frect &bounds)
    : has_bounds_(true), targetset_(false), bounds_(bounds),
      maxspeed_(120.0f)
{
    bounds_.x0 += core::PWIDTH / 2;
    bounds_.y0 += core::PHEIGHT / 2;
    bounds_.x1 -= core::PWIDTH / 2;
    bounds_.y1 -= core::PHEIGHT / 2;

    if (bounds_.x0 > bounds_.x1) {
        bounds_.x0 = bounds_.x1 =
            0.5f * (bounds_.x0 + bounds_.x1);
    }

    if (bounds_.y0 > bounds_.y1) {
        bounds_.y0 = bounds_.y1 =
            0.5f * (bounds_.y0 + bounds_.y1);
    }

    lastpos_ = pos_ = targetpt_ = bounds.nearest(fvec::zero());
}

void camera_system::set_target(const frect &target)
{
    target_ = target;
    if (!targetset_) {
        targetset_ = true;
        fvec center = target.center();
        if (has_bounds_)
            center = bounds_.nearest(center);
        lastpos_ = center;
        pos_ = center;
        targetpt_ = center;
    } else {
        targetpt_ = target.nearest(pos_);
        if (has_bounds_)
            targetpt_ = bounds_.nearest(targetpt_);
    }
}

void camera_system::update()
{
    float maxmove = maxspeed_ * (1e-3 * defs::FRAMETIME);
    lastpos_ = pos_;
    auto delta = targetpt_ - pos_;
    float mag2 = delta.mag2();
    if (mag2 < maxmove * maxmove)
        pos_ = targetpt_;
    else
        pos_ += delta * (maxmove / std::sqrt(mag2));
}

fvec camera_system::get_pos(int reltime) const
{
    return defs::interp(lastpos_, pos_, reltime);
}

}
