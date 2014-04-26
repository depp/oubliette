/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_LEVELMAP_HPP
#define LD_GAME_LEVELMAP_HPP
namespace game {

class levelmap {
private:
    unsigned char *data_;
    int width_;
    int height_;
    bool stop_left_;
    bool stop_right_;
    bool stop_down;
    bool stop_up_;
};

}
#endif
