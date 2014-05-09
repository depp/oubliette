/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "sprite.hpp"
#include "opengl.hpp"
#include "pack.hpp"
#include "surface.hpp"
#include <unordered_map>
#include <cstdio>
#include <string>
#include <SDL.h>
#include <SDL_image.h>

namespace sprite {

sheet::sheet()
    : sprites_(), texture_(0), width_(0), height_(0)
{
    texscale_[0] = texscale_[1] = 0.0f;
}

sheet::sheet(const std::string &dirname, const sprite *sprites)
    : sprites_(), texture_(0), width_(0), height_(0)
{
    texscale_[0] = texscale_[1] = 0.0f;
    std::vector<sdl::surface> images;
    std::vector<std::size_t> spriteimages;
    std::size_t count = 0;

    {
        std::string dirpath(dirname);
        if (!dirpath.empty())
            dirpath += '/';
        std::unordered_map<std::string, std::size_t> imagenames;
        for (int i = 0; sprites[i].name; i++) {
            count++;
            std::string name(sprites[i].name);
            auto x = imagenames.insert(
                std::unordered_map<std::string, std::size_t>::value_type(
                    name, images.size()));
            if (x.second) {
                std::string path = dirpath;
                path += name;
                path += ".png";
                sdl::surface image;
                image.surfptr = IMG_Load(path.c_str());
                if (!image.surfptr) {
                    core::check_sdl_error(HERE);
                    std::fprintf(stderr, "Error: failed to load image: %s\n",
                                 path.c_str());
                    core::die("Failed to load image");
                }
                if (image->format->format == SDL_PIXELFORMAT_ARGB8888) {
                    images.push_back(std::move(image));
                } else {
                    sdl::surface converted;
                    converted.surfptr = SDL_ConvertSurfaceFormat(
                        image.surfptr, SDL_PIXELFORMAT_ARGB8888, 0);
                    if (!converted.surfptr) {
                        core::check_sdl_error(HERE);
                        std::fprintf(
                            stderr, "Error: failed to load image: %s\n",
                            path.c_str());
                        core::die("Failed to load image");
                    }
                    images.push_back(std::move(converted));
                }
            }
            spriteimages.push_back(x.first->second);
        }
    }

    std::vector<pack::size> imagesizes;
    imagesizes.reserve(images.size());
    for (auto &image : images) {
        pack::size sz = { image.surfptr->w, image.surfptr->h };
        imagesizes.push_back(sz);
    }

    pack::packing packing = pack::pack(imagesizes);

    std::fprintf(stderr, "Packing %zu sprites into a %dx%d sheet\n",
                 count, packing.packsize.width, packing.packsize.height);

    sprites_.reserve(count);
    for (std::size_t i = 0; i < count; i++) {
        auto loc = packing.locations[spriteimages[i]];
        rect rect;
        rect.x = sprites[i].x + loc.x;
        rect.y = sprites[i].y + loc.y;
        rect.w = sprites[i].w;
        rect.h = sprites[i].h;
        sprites_.push_back(rect);
    }

    width_ = packing.packsize.width;
    height_ = packing.packsize.height;
    texscale_[0] = 1.0 / width_;
    texscale_[1] = 1.0 / height_;

    sdl::surface surf;
    surf.surfptr = SDL_CreateRGBSurface(
        0, packing.packsize.width, packing.packsize.height, 32,
        0x00ff0000u, 0x0000ff00u, 0x000000ffu, 0xff000000u);
    if (!surf.surfptr) core::die_sdl(HERE, "Failed to pack sprites");

    int r;

    SDL_Surface *main = surf.surfptr;
    for (std::size_t i = 0; i < images.size(); i++) {
        SDL_Surface *surf = images[i].surfptr;
        r = SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);
        if (r) core::die_sdl(HERE, "Failed to pack sprites");

        SDL_Rect rect;
        rect.x = packing.locations[i].x;
        rect.y = packing.locations[i].y;
        rect.w = surf->w;
        rect.h = surf->h;

        r = SDL_BlitSurface(surf, nullptr, main, &rect);
        if (r) core::die_sdl(HERE, "Failed to pack sprites");
    }

    r = SDL_LockSurface(main);
    if (r) core::die_sdl(HERE, "Failed to pack sprites");

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        packing.packsize.width,
        packing.packsize.height,
        0,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        main->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

sheet::sheet(sheet &&other)
    : sprites_(std::move(other.sprites_)), texture_(other.texture_),
      width_(other.width_), height_(other.height_)
{
    other.sprites_.clear();
    other.texture_ = 0;
    other.width_ = 0;
    other.height_ = 0;
    other.texscale_[0] = 0.0f;
    other.texscale_[1] = 0.0f;
}

sheet &sheet::operator=(sheet &&other)
{
    if (this == &other)
        return *this;
    if (texture_ != 0)
        glDeleteTextures(1, &texture_);

    sprites_ = std::move(other.sprites_);
    texture_ = other.texture_;
    width_ = other.width_;
    height_ = other.height_;
    texscale_[0] = other.texscale_[0];
    texscale_[1] = other.texscale_[1];
    other.sprites_.clear();
    other.texture_ = 0;
    other.width_ = 0;
    other.height_ = 0;
    other.texscale_[0] = 0.0f;
    other.texscale_[1] = 0.0f;

    return *this;
}

sheet::~sheet()
{
    if (texture_ != 0)
        glDeleteTextures(1, &texture_);
}

}
