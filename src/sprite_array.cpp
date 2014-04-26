/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "sprite.hpp"
namespace sprite {

array::array()
{
}

array::array(array &&other)
    : array_(std::move(other.array_))
{
}

array::~array()
{
}

void array::clear()
{
    array_.clear();
}

void array::add(rect tex, int x, int y)
{
    short (*data)[4] = array_.insert(6);

    short tx0 = tex.x, tx1 = tex.x + tex.w;
    short ty1 = tex.y, ty0 = tex.y + tex.h;
    short vx0 = x, vx1 = x + tex.w;
    short vy0 = y, vy1 = y + tex.h;

    data[0][0] = vx0; data[0][1] = vy0; data[0][2] = tx0; data[0][3] = ty0;
    data[1][0] = vx1; data[1][1] = vy0; data[1][2] = tx1; data[1][3] = ty0;
    data[2][0] = vx0; data[2][1] = vy1; data[2][2] = tx0; data[2][3] = ty1;
    data[3][0] = vx0; data[3][1] = vy1; data[3][2] = tx0; data[3][3] = ty1;
    data[4][0] = vx1; data[4][1] = vy0; data[4][2] = tx1; data[4][3] = ty0;
    data[5][0] = vx1; data[5][1] = vy1; data[5][2] = tx1; data[5][3] = ty1;
}

void array::add(rect tex, int x, int y, orientation orient)
{
    short (*data)[4] = array_.insert(6);

    short tx0 = tex.x, tx1 = tex.x + tex.w;
    short ty1 = tex.y, ty0 = tex.y + tex.h;

    data[0][2] = tx0; data[0][3] = ty0;
    data[1][2] = tx1; data[1][3] = ty0;
    data[2][2] = tx0; data[2][3] = ty1;
    data[3][2] = tx0; data[3][3] = ty1;
    data[4][2] = tx1; data[4][3] = ty0;
    data[5][2] = tx1; data[5][3] = ty1;

    short vx[4], vy[4];
    switch (orient) {
    case orientation::NORMAL:
        vx[0] = vx[2] = x;
        vx[1] = vx[3] = x + tex.w;
        vy[0] = vy[1] = y;
        vy[2] = vy[3] = y + tex.h;
        break;

    case orientation::ROTATE_90:
        vx[2] = vx[3] = x;
        vx[0] = vx[1] = x + tex.h;
        vy[2] = vy[0] = y;
        vy[3] = vy[1] = y + tex.w;
        break;

    case orientation::ROTATE_180:
        vx[3] = vx[1] = x;
        vx[2] = vx[0] = x + tex.w;
        vy[3] = vy[2] = y;
        vy[1] = vy[0] = y + tex.h;
        break;

    case orientation::ROTATE_270:
        vx[1] = vx[0] = x;
        vx[3] = vx[2] = x + tex.h;
        vy[1] = vy[3] = y;
        vy[0] = vy[2] = y + tex.w;
        break;

    case orientation::FLIP_VERTICAL:
        vx[0] = vx[2] = x;
        vx[1] = vx[3] = x + tex.w;
        vy[0] = vy[1] = y + tex.h;
        vy[2] = vy[3] = y;
        break;

    case orientation::TRANSPOSE_2:
        vx[2] = vx[3] = x;
        vx[0] = vx[1] = x + tex.h;
        vy[2] = vy[0] = y + tex.w;
        vy[3] = vy[1] = y;
        break;

    case orientation::FLIP_HORIZONTAL:
        vx[3] = vx[1] = x;
        vx[2] = vx[0] = x + tex.w;
        vy[3] = vy[2] = y + tex.h;
        vy[1] = vy[0] = y;
        break;

    case orientation::TRANSPOSE:
        vx[1] = vx[0] = x;
        vx[3] = vx[2] = x + tex.h;
        vy[1] = vy[3] = y + tex.w;
        vy[0] = vy[2] = y;
        break;
    }

    data[0][0] = vx[0]; data[0][1] = vy[0];
    data[1][0] = vx[1]; data[1][1] = vy[1];
    data[2][0] = vx[2]; data[2][1] = vy[2];
    data[3][0] = vx[2]; data[3][1] = vy[2];
    data[4][0] = vx[1]; data[4][1] = vy[1];
    data[5][0] = vx[3]; data[5][1] = vy[3];
}

void array::upload(GLuint usage)
{
    array_.upload(usage);
}

void array::set_attrib(GLint attrib)
{
    array_.set_attrib(attrib);
}

}
