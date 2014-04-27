/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_IMAGE_HPP
#define LD_IMAGE_HPP
#include <string>
#include "opengl.hpp"
namespace image {

struct bitmap {
    unsigned char *data;
    int width;
    int height;
    int rowbytes;

    bitmap();
    bitmap(const bitmap &) = delete;
    bitmap(bitmap &&other);
    ~bitmap();
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

    texture();

    /// Load an image as a 2-dimensional texture.
    /// The image is padded so its dimensions are powers of two.
    static texture load(const std::string &path);

    /// Load an image as a 1-dimensional texture.
    /// The image is padded so its dimensions are powers of two.
    static texture load_1d(const std::string &path);
};

}
#endif
