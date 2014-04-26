/* Copyright 2013-2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_SPRITE_HPP
#define LD_SPRITE_HPP

#include "array.hpp"
#include "opengl.hpp"
#include <array>
#include <cstdlib>
#include <vector>
#include <string>

namespace sprite {

/// Orthogonal orientations for 2D sprites.
enum class orientation {
    NORMAL, ROTATE_90, ROTATE_180, ROTATE_270,
    FLIP_VERTICAL, TRANSPOSE_2, FLIP_HORIZONTAL, TRANSPOSE
};

/// Compose two orientations.
orientation operator*(orientation x, orientation y);

/// A 2D integer rectangle.
struct rect {
    short x, y, w, h;
};

/// A sprite to add to a sprite sheet.
struct sprite {
    const char *name;
    short x, y, w, h;
};

/// A sprite sheet.
class sheet {
private:
    std::vector<rect> sprites_;
    GLuint texture_;
    int width_, height_;
    float texscale_[2];

public:
    sheet();
    sheet(const std::string &dirname, const sprite *sprites);
    sheet(const sheet &other) = delete;
    sheet(sheet &&other);
    ~sheet();
    sheet &operator=(const sheet &other) = delete;
    sheet &operator=(sheet &&other);

    /// Get the texture object containing the sprites.
    GLuint texture() const { return texture_; }
    /// Get the width of the texture.
    int width() const { return width_; }
    /// Get the height of the texture.
    int height() const { return height_; }
    /// Get the factor to convert pixel coordinates to texture coordinates.
    const float *texscale() const { return texscale_; }
    /// Get the rectangle containing the given sprite.
    rect get(int index) const { return sprites_.at(index); }
};

// Array of sprite rectangles with texture coordinates.
// Draw with GL_TRIANGLES.
class array {
private:
    ::array::array<short[4]> array_;

public:
    array();
    array(const array &other) = delete;
    array(array &&other);
    ~array();
    array &operator=(const array &other) = delete;
    array &operator=(array &&other) = delete;

    /// Clear the array.
    void clear();
    /// Add a sprite (tex) at the given lower-left coordinate.
    void add(rect tex, int x, int y);
    /// Add a sprite (tex) at the given lower-left coordinate.
    void add(rect tex, int x, int y, orientation orient);
    /// Upload the array data.
    void upload(GLuint usage);
    /// Bind the OpenGL attribute.
    void set_attrib(GLint attrib);
    /// Get the number of vertexes.
    int size() const { return array_.size(); }
};

}

#endif
