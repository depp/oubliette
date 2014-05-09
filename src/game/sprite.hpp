/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_SPRITE_HPP
#define LD_GAME_SPRITE_HPP
namespace sprite {
struct sprite;
}
namespace graphics {

// Define the "sprite" and "ui" enumerations.
#include "sprite_enum.hpp"

/// A sprite from any category.
class anysprite {
private:
    int index;

public:
    anysprite() { }
    anysprite(sprite s) : index(static_cast<int>(s)) { }
    anysprite(ui s) : index(static_cast<int>(s) + SPRITE_COUNT) { }
    operator int() const { return index; }
};

ui treasure_sprite(int which, int state);
extern const ::sprite::sprite SPRITES[];

};
#endif
