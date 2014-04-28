/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "graphics.hpp"
#include "state.hpp"
#include "defs.hpp"
#include "../defs.hpp"
#include "../rand.hpp"
namespace graphics {
using game::vec2;

static struct ::sprite::sprite SPRITES[] = {
#include "sprite_array.hpp"
    { nullptr, 0, 0, 0, 0 }
};

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

sprite treasure_sprite(int which, int state)
{
    if (which < 0 || which >= 3 || state < 0 || state >= 5)
        core::die("Invalid treasure");
    static const sprite ARR[3][5] = {
        { sprite::CIRCLE1,
          sprite::CIRCLE2,
          sprite::CIRCLE3,
          sprite::CIRCLE4,
          sprite::CIRCLE5 },
        { sprite::STAR1,
          sprite::STAR2,
          sprite::STAR3,
          sprite::STAR4,
          sprite::STAR5 },
        { sprite::DIAMOND1,
          sprite::DIAMOND2,
          sprite::DIAMOND3,
          sprite::DIAMOND4,
          sprite::DIAMOND5 }
    };
    return ARR[which][state];
}

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

// ======================================================================

common_data::common_data()
    : sprite("sprite", "sprite"),
      tv("tv", "tv"),
      plain("plain", "plain"),
      text("sprite", "text")
{ }

// ======================================================================

sprite_data::sprite_data()
    : sheet("sprite", SPRITES)
{ }

void sprite_data::clear()
{
    array.clear();
    array2.clear();
}

void sprite_data::upload()
{
    array.upload(GL_DYNAMIC_DRAW);
    array2.upload(GL_DYNAMIC_DRAW);
}

void sprite_data::draw(const common_data &com)
{
    glUseProgram(com.sprite.prog());
    glEnableVertexAttribArray(com.sprite->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sheet.texture());

    glUniform2fv(com.sprite->u_texscale, 1, sheet.texscale());
    glUniform1i(com.sprite->u_texture, 0);

    if (!array.empty()) {
        array.set_attrib(com.sprite->a_vert);
        glUniform4fv(com.sprite->u_vertxform, 1, com.xform_world);
        glDrawArrays(GL_TRIANGLES, 0, array.size());
    }

    if (!array2.empty()) {
        array2.set_attrib(com.sprite->a_vert);
        glUniform4fv(com.sprite->u_vertxform, 1, com.xform_screen);
        glDrawArrays(GL_TRIANGLES, 0, array2.size());
    }

    glDisableVertexAttribArray(com.sprite->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);
}

// ======================================================================

background_data::background_data()
{ }

void background_data::clear()
{
    array.clear();
}

void background_data::upload()
{
    if (bgtex.tex == 0)
        return;

    ::sprite::rect r = {
        0, 0,
        bgtex.iwidth, bgtex.iheight
    };
    array.add(r, 0, 0);
    array.upload(GL_DYNAMIC_DRAW);
    core::check_gl_error(HERE);
}

void background_data::draw(const common_data &com)
{
    if (bgtex.tex == 0)
        return;

    glUseProgram(com.sprite.prog());
    glEnableVertexAttribArray(com.sprite->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bgtex.tex);

    glUniform4fv(com.sprite->u_vertxform, 1, com.xform_world);
    glUniform2fv(com.sprite->u_texscale, 1, bgtex.scale);
    glUniform1i(com.sprite->u_texture, 0);
    array.set_attrib(com.sprite->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(com.sprite->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);
}

void background_data::set_level(const std::string &name)
{
    if (bgtex.tex) {
        glDeleteTextures(1, &bgtex.tex);
        bgtex.tex = 0;
    }

    if (!name.empty()) {
        std::string fullpath("level/");
        fullpath += name;
        fullpath += ".png";
        bgtex = image::texture::load(fullpath);
    }
}

// ======================================================================

void selection_data::clear()
{
    array.clear();
}

void selection_data::upload()
{
    if (array.empty())
        return;

    array.upload(GL_DYNAMIC_DRAW);
}

void selection_data::draw(const common_data &com)
{
    if (array.empty())
        return;

    glUseProgram(com.plain.prog());
    glEnableVertexAttribArray(com.plain->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glUniform4fv(com.plain->u_vertxform, 1, com.xform_world);
    glUniform4f(
        com.plain->u_color,
        0.4f, 0.0f, 0.4f, 0.0);
    array.set_attrib(com.plain->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(com.plain->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);
}

// ======================================================================

font_data::font_data()
    : dirty(false)
{
    tex = image::texture::load("font/terminus.png");
}

void font_data::clear()
{
    if (!array.empty())
        dirty = true;
    array.clear();
    blocks.clear();
}

void font_data::upload()
{
    if (dirty) {
        array.upload(GL_DYNAMIC_DRAW);
        dirty = false;
    }
}

void font_data::draw(const common_data &com)
{
    if (array.empty())
        return;

    glUseProgram(com.text.prog());
    glEnableVertexAttribArray(com.text->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.tex);

    glUniform2f(com.text->u_texscale, 1.0f/16.0f, 1.0f/16.0f);
    glUniform1i(com.text->u_texture, 0);

    array.set_attrib(com.text->a_vert);
    glUniform4fv(com.text->u_vertxform, 1, com.xform_screen);

    int pos = 0;
    for (auto i = blocks.begin(), e = blocks.end(); i != e; i++) {
        glUniform4fv(com.text->u_color, 1, i->color);
        glDrawArrays(GL_TRIANGLES, pos, i->vertcount);
        pos += i->vertcount;
    }

    glDisableVertexAttribArray(com.text->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);
}

int font_data::add_text(const std::string &text, int x, int y)
{
    int xpos = x, ypos = y;
    int vertcount = 0;
    for (auto i = text.begin(), e = text.end(); i != e; i++) {
        unsigned char c = *i;
        if (c == ' ') {
            xpos += 8;
            continue;
        } else if (c == '\n') {
            xpos = x;
            ypos -= 16;
            continue;
        }

        short x0 = xpos, x1 = xpos + 8, y0 = ypos - 16, y1 = ypos;
        short u0 = (c & 15), u1 = u0 + 1, v1 = (c >> 4), v0 = v1 + 1;
        auto d = array.insert(6);
        d[0][0] = x0; d[0][1] = y0; d[0][2] = u0; d[0][3] = v0;
        d[1][0] = x1; d[1][1] = y0; d[1][2] = u1; d[1][3] = v0;
        d[2][0] = x0; d[2][1] = y1; d[2][2] = u0; d[2][3] = v1;
        d[3][0] = x0; d[3][1] = y1; d[3][2] = u0; d[3][3] = v1;
        d[4][0] = x1; d[4][1] = y0; d[4][2] = u1; d[4][3] = v0;
        d[5][0] = x1; d[5][1] = y1; d[5][2] = u1; d[5][3] = v1;
        xpos += 8;
        vertcount++;
    }

    if (vertcount == 0)
        return -1;

    dirty = true;
    block b;
    b.vertcount = vertcount * 6;
    b.color[0] = 1.0f;
    b.color[1] = 0.0f;
    b.color[2] = 1.0f;
    b.color[3] = 1.0f;
    blocks.push_back(b);

    return blocks.size() - 1;
}

void font_data::set_color(int block, const float color[4])
{
    if (block < 0 || (std::size_t)block >= blocks.size())
        return;
    auto &b = blocks[block];
    for (int i = 0; i < 4; i++)
        b.color[i] = color[i];
}

// ======================================================================

scale_data::scale_data()
{
    width = round_up_pow2(core::PWIDTH);
    height = round_up_pow2(core::PHEIGHT);
    scale[0] = 1.0f / width;
    scale[1] = 1.0f / height;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbuf);
    glBindFramebuffer(GL_FRAMEBUFFER, fbuf);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        core::die("Cannot render to framebuffer");

    float (*data)[4] = array.insert(6);
    float u = (float) core::PWIDTH, v = (float) core::PHEIGHT;
    data[0][0] = -1.0f; data[0][1] = -1.0f; data[0][2] = 0; data[0][3] = 0;
    data[1][0] = +1.0f; data[1][1] = -1.0f; data[1][2] = u; data[1][3] = 0;
    data[2][0] = -1.0f; data[2][1] = +1.0f; data[2][2] = 0; data[2][3] = v;
    data[3][0] = -1.0f; data[3][1] = +1.0f; data[3][2] = 0; data[3][3] = v;
    data[4][0] = +1.0f; data[4][1] = -1.0f; data[4][2] = u; data[4][3] = 0;
    data[5][0] = +1.0f; data[5][1] = +1.0f; data[5][2] = u; data[5][3] = v;
    array.upload(GL_STATIC_DRAW);

    texpattern = image::texture::load("tv/pattern.png");
    texbanding = image::texture::load_1d("tv/banding.png");
    texnoise = image::texture::load("tv/noise.png");

    core::check_gl_error(HERE);
}

void scale_data::begin()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbuf);
    glViewport(0, 0, width, height);
    glClearColor(PALETTE[0][0], PALETTE[0][1], PALETTE[0][2], PALETTE[0][3]);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, core::PWIDTH, core::PHEIGHT);

    core::check_gl_error(HERE);
}

void scale_data::end(const common_data &com)
{
    unsigned x = rng::global.next();
    float offsets[4] = {
        (float)((x >>  0) & 255),
        (float)((x >>  8) & 255),
        (float)((x >> 16) & 255),
        (float)((x >> 24) & 255)
    };

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, core::IWIDTH, core::IHEIGHT);
    glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(com.tv.prog());
    glEnableVertexAttribArray(com.tv->a_vert);
    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texpattern.tex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, texbanding.tex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texnoise.tex);

    glUniform1i(com.tv->u_picture, 0);
    glUniform1i(com.tv->u_pattern, 1);
    glUniform1i(com.tv->u_banding, 2);
    glUniform1i(com.tv->u_noise, 3);
    glUniform4fv(com.tv->u_noiseoffset, 1, offsets);
    glUniform2fv(com.tv->u_texscale, 1, scale);
    glUniform4fv(com.tv->u_color, 1, color);
    array.set_attrib(com.tv->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(com.tv->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);
}

// ======================================================================

system::system()
    : camera_(vec2::zero())
{ }

system::~system()
{ }

void system::begin()
{
    sprite_.clear();
    background_.clear();
    selection_.clear();
    for (int i = 0; i < 4; i++)
        scale_.color[i] = 0.0f;
}

void system::end()
{
    sprite_.upload();
    background_.upload();
    selection_.upload();
    font_.upload();
}

void system::draw()
{
    common_.xform_world[0] = 2.0 / core::PWIDTH;
    common_.xform_world[1] = 2.0 / core::PHEIGHT;
    common_.xform_world[2] = camera_.x * (-2.0 / core::PWIDTH);
    common_.xform_world[3] = camera_.y * (-2.0 / core::PHEIGHT);
    common_.xform_screen[0] = 2.0 / core::PWIDTH;
    common_.xform_screen[1] = 2.0 / core::PHEIGHT;
    common_.xform_screen[2] = -1.0f;
    common_.xform_screen[3] = -1.0f;

    scale_.begin();
    background_.draw(common_);
    selection_.draw(common_);
    sprite_.draw(common_);
    font_.draw(common_);
    scale_.end(common_);
}

void system::set_level(const std::string &path)
{
    background_.set_level(path);
}

void system::add_sprite(sprite sp, vec2 pos,
                        ::sprite::orientation orient,
                        bool screen_relative)
{
    auto &arr = screen_relative ? sprite_.array2 : sprite_.array;
    arr.add(
        sprite_.sheet.get(static_cast<int>(sp)),
        (int)std::floor(pos.x),
        (int)std::floor(pos.y),
        orient);
}

void system::set_camera_pos(vec2 target)
{
    camera_ = vec2(std::floor(target.x), std::floor(target.y));
}

void system::set_selection(game::irect rect)
{
    selection_.array.clear();
    auto d = selection_.array.insert(6);
    d[0][0] = rect.x0; d[0][1] = rect.y0;
    d[1][0] = rect.x1; d[1][1] = rect.y0;
    d[2][0] = rect.x0; d[2][1] = rect.y1;
    d[3][0] = rect.x0; d[3][1] = rect.y1;
    d[4][0] = rect.x1; d[4][1] = rect.y0;
    d[5][0] = rect.x1; d[5][1] = rect.y1;
}

void system::set_blend_color(const float color[4])
{
    for (int i = 0; i < 4; i++)
        scale_.color[i] = color[i];
}

void system::clear_text()
{
    font_.clear();
}

int system::add_text(const std::string &text, int x, int y)
{
    return font_.add_text(text, x, y);
}

void system::set_text_color(int block, const float color[4])
{
    font_.set_color(block, color);
}

}
