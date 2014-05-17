/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_COLOR_HPP
#define LD_GAME_COLOR_HPP
namespace graphics {

struct color {
    float v[4];

    /// The transparent color.
    static color transparent();
    /// A color from DB's 16-color palette.
    static color palette(int index);
    /// Produce a fade between this color and black.
    /// 1.0 produces the original color, 0.0 produces transparent.
    color fade(float alpha) const;
    /// Fade between two colors.
    static color blend(const color &a, const color &b, float t);
};

}
#endif
