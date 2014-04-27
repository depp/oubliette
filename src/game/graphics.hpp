/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_GRAPHICS_HPP
#define LD_GAME_GRAPHICS_HPP
#include <memory>
#include "../sprite.hpp"
#include "../shader.hpp"
#include "../image.hpp"
#include "vec.hpp"
namespace graphics {
class state;

// Define the "sprite" enumeration
#include "sprite_enum.hpp"

/// The shader programs.
struct program_data {
    shader::program<shader::sprite> sprite;
    shader::program<shader::tv> tv;

    program_data();
};

/// The foreground sprites.
struct sprite_data {
    ::sprite::sheet sheet;
    ::sprite::array array;

    sprite_data();
    void clear();
    void upload();
    void draw(const program_data &prog, game::vec2 camera);
};

/// The level background.
struct background_data {
    image::texture bgtex;
    ::sprite::array array;

    background_data();
    void clear();
    void upload();
    void draw(const program_data &prog, game::vec2 camera);
    void set_level(const std::string &path);
};

/// Pixel scaling data.
struct scale_data {
    GLuint tex;
    GLuint fbuf;
    array::array<float[4]> array;
    int width, height;
    image::texture texpattern;
    image::texture texbanding;
    image::texture texnoise;
    float scale[2];

    scale_data();
    void begin();
    void end(const program_data &prog);
};

/// The graphics system.
class system {
private:
    program_data prog_;
    game::vec2 camera_;
    sprite_data sprite_;
    background_data background_;
    scale_data scale_;

public:
    system();
    ~system();

    /// Begin updates to the graphics state.
    void begin();
    /// End updates to the graphics data.
    void end();
    /// Draw the world, at a time relative to the last update.
    void draw();
    /// Set the current level.
    void set_level(const std::string &path);
    /// Add a sprite to the screen.
    void add_sprite(sprite sp, game::vec2 pos, ::sprite::orientation orient);
    /// Set the camera target.
    void set_camera_pos(game::vec2 target);
};

}
#endif
