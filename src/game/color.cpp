/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "color.hpp"
#include "base/defs.hpp"
namespace graphics {

color color::transparent()
{
    color c;
    for (int i = 0; i < 4; i++)
        c.v[i] = 0.0f;
    return c;
}

color color::palette(int index)
{
    static const unsigned char PALETTE[16][3] = {
        {  20,  12,  28 },
        {  68,  36,  52 },
        {  48,  52, 109 },
        {  78,  74,  78 },
        { 133,  76,  48 },
        {  52, 101,  36 },
        { 208,  70,  72 },
        { 117, 113,  97 },
        {  89, 125, 206 },
        { 210, 125,  44 },
        { 133, 149, 161 },
        { 109, 170,  44 },
        { 210, 170, 153 },
        { 109, 194, 202 },
        { 218, 212,  94 },
        { 222, 238, 214 }
    };
    if (index < 0 || index >= 16)
        core::die("Invalid palette index");
    color c;
    for (int i = 0; i < 3; i++)
        c.v[i] = (float)PALETTE[index][i] * (float)(1.0f / 255.0f);
    c.v[3] = 1.0f;
    return c;
}

color color::fade(float alpha) const
{
    if (alpha > 1.0f)
        alpha = 1.0f;
    else if (alpha < 0.0f)
        alpha = 0.0f;
    color c;
    for (int i = 0; i < 4; i++)
        c.v[i] = v[i] * alpha;
    return c;
}

color color::blend(const color &a, const color &b, float t)
{
    if (t > 1.0f)
        t = 1.0f;
    else if (t < 0.0f)
        t = 0.0f;
    color c;
    for (int i = 0; i < 4; i++)
        c.v[i] = a.v[i] * (1.0f - t) + b.v[i] * t;
    return c;
}

}
