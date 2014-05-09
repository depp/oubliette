/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_COLOR_HPP
#define LD_GAME_COLOR_HPP
namespace graphics {

// DB's 16-color palette, plus transparent at #16
extern const float PALETTE[17][4];

void blend(float out[4], const float a[4], float alpha);
void blend(float out[4], const float a[4], float aalpha,
           const float b[4], float balpha, float t);
void blend(float out[4], const float a[4], const float b[4], float t);
void blend(float out[4], int a, float alpha);
void blend(float out[4], int a, float aalpha, int b, float balpha, float t);
void blend(float out[4], int a, int b, float t);

}
#endif
