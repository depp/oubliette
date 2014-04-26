/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "image.hpp"
#include "defs.hpp"
#include "surface.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <cstdio>
namespace image {

static sdl::surface load_image(const std::string &path)
{
    sdl::surface image;

    image.surfptr = IMG_Load(path.c_str());
    if (!image.surfptr) {
        core::check_sdl_error(HERE);
        std::fprintf(stderr, "Error: failed to load image: %s\n",
                     path.c_str());
        core::die("Failed to load image");
    }

    return image;
}

static int round_up_pow2(int x)
{
    unsigned v = x - 1;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

bitmap::bitmap(bitmap &&other)
    : data(other.data), width(other.width), height(other.height),
      rowbytes(other.rowbytes)
{
    other.data = nullptr;
    other.width = 0;
    other.height = 0;
    other.rowbytes = 0;
}

bitmap &bitmap::operator=(bitmap &&other)
{
    data = other.data;
    width = other.width;
    height = other.height;
    rowbytes = other.rowbytes;
    other.data = nullptr;
    other.width = 0;
    other.height = 0;
    other.rowbytes = 0;
    return *this;
}

void bitmap::alloc(int w, int h)
{
    std::free(data);
    data = nullptr;
    width = 0;
    height = 0;
    rowbytes = 0;

    int rb = ((unsigned)w + 15) & ~15u;
    if (w < 1 || w > 0x8000 || h < 1 || h > 0x8000)
        core::die("bitmap too large");

    data = std::malloc(rb * h);
    if (data == nullptr)
        core::die_alloc();

    width = w;
    height = h;
    rowbytes = rb;
}

bitmap bitmap::load(const std::string &path)
{
    sdl::surface image = load_image(path);

    int r = SDL_LockSurface(image.surfptr);
    if (r) core::die_sdl(HERE, "Failed to load image");

    int iw = image->w, ih = image->h;
    bitmap bmap;
    bmap.alloc(iw, ih);
    const unsigned char *ip = static_cast<unsigned char *>(image->pixels);
    unsigned char *op = static_cast<unsigned char *>(bmap.data);
    unsigned irb = image->pitch, orb = bmap.rowbytes;
    for (int y = 0; y < ih; y++) {
        const unsigned *irow =
            reinterpret_cast<const unsigned *>(ip + irb * y);
        unsigned char *orow = op + orb * y;
        for (int x = 0; x < iw; x++) {
            orow[x] = irow[x] >> 24;
        }
    }

    return bmap;
}

texture texture::load(const std::string &path)
{
    sdl::surface image = load_image(path);
    texture tex;

    int r = SDL_LockSurface(image.surfptr);
    if (r) core::die_sdl(HERE, "Failed to load image");

    tex.iwidth = image->w;
    tex.iheight = image->h;
    tex.twidth = round_up_pow2(tex.iwidth);
    tex.theight = round_up_pow2(tex.iheight);
    tex.scale[0] = 1.0 / tex.twidth;
    tex.scale[1] = 1.0 / tex.theight;

    glGenTextures(1, &tex.tex);
    glBindTexture(GL_TEXTURE_2D, tex.tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        tex.twidth,
        tex.theight,
        0,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        nullptr);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        tex.iwidth,
        tex.iheight,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        image->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 0x00ff0000u, 0x0000ff00u, 0x000000ff, 0xff000000u

    return tex;
}

}
