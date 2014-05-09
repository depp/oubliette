/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "color.hpp"
#include "../defs.hpp"
namespace graphics {

#define c(r, g, b) { (r)/255.0f, (g)/255.0f, (b)/255.0f, 1.0f }
const float PALETTE[17][4] = {
    c(20, 12, 28),
    c(68, 36, 52),
    c(48, 52, 109),
    c(78, 74, 78),
    c(133, 76, 48),
    c(52, 101, 36),
    c(208, 70, 72),
    c(117, 113, 97),
    c(89, 125, 206),
    c(210, 125, 44),
    c(133, 149, 161),
    c(109, 170, 44),
    c(210, 170, 153),
    c(109, 194, 202),
    c(218, 212, 94),
    c(222, 238, 214),
    { 0, 0, 0, 0 }
};
#undef c

void blend(float out[4], const float a[4], float alpha)
{
    for (int i = 0; i < 4; i++)
        out[i] = a[i] * alpha;
}

void blend(float out[4], const float a[4], float aalpha,
           const float b[4], float balpha, float t)
{
    if (t > 1.0f)
        t = 1.0f;
    else if (t < 0.0f)
        t = 0.0f;
    for (int i = 0; i < 4; i++)
        out[i] = a[i] * (1.0f - t) * aalpha + b[i] * t * balpha;
}

void blend(float out[4], const float a[4], const float b[4], float t)
{
    blend(out, a, 1.0f, b, 1.0f, t);
}

static const float *get_palette(int n)
{
    if (n >= 0 && n <= 16)
        return PALETTE[n];
    core::die("Palette out of bounds");
}

void blend(float out[4], int a, float alpha)
{
    blend(out, get_palette(a), alpha);
}

void blend(float out[4], int a, float aalpha, int b, float balpha, float t)
{
    blend(out, get_palette(a), aalpha, get_palette(b), balpha, t);
}

void blend(float out[4], int a, int b, float t)
{
    blend(out, get_palette(a), 1.0f, get_palette(b), 1.0f, t);
}

}
