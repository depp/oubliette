/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "sprite.hpp"
#include "../defs.hpp"
#include "../sprite.hpp"
namespace graphics {

const struct ::sprite::sprite SPRITES[] = {
#include "sprite_array.hpp"
    { nullptr, 0, 0, 0, 0 }
};

ui treasure_sprite(int which, int state)
{
    if (which < 0 || which >= 3 || state < 0 || state >= 4)
        core::die("Invalid treasure");
    static const ui ARR[3][4] = {
        { ui::TREASURE01, ui::TREASURE11, ui::TREASURE21, ui::TREASURE31 },
        { ui::TREASURE02, ui::TREASURE12, ui::TREASURE22, ui::TREASURE32 },
        { ui::TREASURE03, ui::TREASURE13, ui::TREASURE23, ui::TREASURE33 }
    };
    return ARR[which][state];
}

}
