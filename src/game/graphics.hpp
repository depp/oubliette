/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_GRAPHICS_HPP
#define LD_GAME_GRAPHICS_HPP
#include <memory>
#include "base/sprite.hpp"
#include "base/shader.hpp"
#include "base/image.hpp"
#include "sprite.hpp"
#include "base/vec.hpp"
#include "color.hpp"
namespace graphics {
class state;

/// The shader commons.
struct common_data {
    shader::program<shader::sprite> sprite;
    shader::program<shader::tv> tv;
    shader::program<shader::plain> plain;
    shader::program<shader::text> text;

    // Vertex transformation uniform.
    float xform_world[4];
    float xform_screen[4];

    common_data();
};

/// The foreground sprites.
struct sprite_data {
    ::sprite::sheet sheet;
    ::sprite::array array;
    ::sprite::array array2;

    sprite_data();
    void clear();
    void upload();
    void draw(const common_data &com);
};

/// The level background.
struct background_data {
    image::texture bgtex;
    ::sprite::array array;

    background_data();
    void clear();
    void upload();
    void draw(const common_data &com);
    void set_level(const std::string &name);
};

/// Editor selection data.
struct selection_data {
    array::array<short[2]> array;

    void clear();
    void upload();
    void draw(const common_data &com);
};

/// Font rendering data.
struct font_data {
    struct block {
        int vertcount;
        color text_color;
    };

    image::texture tex;
    array::array<short[4]> array;
    bool dirty;
    std::vector<block> blocks;

    font_data();
    void clear();
    void upload();
    void draw(const common_data &com);
    int add_text(const std::string &text, int x, int y);
    void set_color(int block, const color &text_color);
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
    color blend_color;

    scale_data();
    void begin();
    void end(const common_data &com);
};

/// The graphics system.
class system {
private:
    common_data common_;
    ivec camera_;
    sprite_data sprite_;
    background_data background_;
    selection_data selection_;
    font_data font_;
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
    void add_sprite(anysprite sp, ivec pos,
                    ::sprite::orientation orient,
                    bool screen_relative=false);
    /// Set the camera target.
    void set_camera_pos(ivec target);
    /// Set the editor's selection.
    void set_selection(const irect &rect);
    /// Set the blend effect color.
    void set_blend_color(const color &blend_color);
    /// Clear the text buffer.
    void clear_text();
    /// Add text to the buffer, return the block index.
    int add_text(const std::string &text, int x, int y);
    /// Set text block color.
    void set_text_color(int block, const color &text_color);
};

}
#endif
