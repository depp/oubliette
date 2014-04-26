/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_IMAGE_HPP
#define LD_IMAGE_HPP

#include <string>
#include <cstdlib>
#include "opengl.hpp"
namespace image {

struct bitmap {
    void *data;
    int width;
    int height;
    int rowbytes;

    bitmap() : data(nullptr), width(0), height(0), rowbytes(0) { }
    bitmap(const bitmap &) = delete;
    bitmap(bitmap &&other);
    ~bitmap() { std::free(data); }
    bitmap &operator=(const bitmap &) = delete;
    bitmap &operator=(bitmap &&other);

    void alloc(int w, int h);

    static bitmap load(const std::string &path);
};

struct texture {
    GLuint tex;
    short iwidth;
    short iheight;
    short twidth;
    short theight;
    float scale[2];

    texture()
        : tex(0),
          iwidth(0.0f), iheight(0.0f),
          twidth(0.0f), theight(0.0f)
    { }

    static texture load(const std::string &path);
};

}

#endif
