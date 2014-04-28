/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include <assert.h>
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

// ======================================================================

common_data::common_data()
    : sprite("sprite", "sprite"),
      tv("tv", "tv"),
      plain("plain", "plain")
{ }

// ======================================================================

sprite_data::sprite_data()
    : sheet("sprite", SPRITES)
{ }

void sprite_data::clear()
{
    array.clear();
}

void sprite_data::upload()
{
    array.upload(GL_DYNAMIC_DRAW);
}

void sprite_data::draw(const common_data &com)
{
    glUseProgram(com.sprite.prog());
    glEnableVertexAttribArray(com.sprite->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sheet.texture());

    glUniform4fv(com.sprite->u_vertxform, 1, com.xform);
    glUniform2fv(com.sprite->u_texscale, 1, sheet.texscale());
    glUniform1i(com.sprite->u_texture, 0);
    array.set_attrib(com.sprite->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

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

    glUniform4fv(com.sprite->u_vertxform, 1, com.xform);
    glUniform2fv(com.sprite->u_texscale, 1, bgtex.scale);
    glUniform1i(com.sprite->u_texture, 0);
    array.set_attrib(com.sprite->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(com.sprite->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);
}

void background_data::set_level(const std::string &path)
{
    std::string fullpath("level/");
    fullpath += path;
    fullpath += ".png";

    if (bgtex.tex) {
        glDeleteTextures(1, &bgtex.tex);
        bgtex.tex = 0;
    }
    bgtex = image::texture::load(fullpath);
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

    glUniform4fv(com.plain->u_vertxform, 1, com.xform);
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
    glClearColor(
        20.0f / 255.0f,
        12.0f / 255.0f,
        28.0f / 255.0f,
        0.0f);
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
}

void system::end()
{
    sprite_.upload();
    background_.upload();
    selection_.upload();
}

void system::draw()
{
    common_.xform[0] = 2.0 / core::PWIDTH;
    common_.xform[1] = 2.0 / core::PHEIGHT;
    common_.xform[2] = camera_.x * (-2.0 / core::PWIDTH);
    common_.xform[3] = camera_.y * (-2.0 / core::PHEIGHT);

    scale_.begin();
    background_.draw(common_);
    selection_.draw(common_);
    sprite_.draw(common_);
    scale_.end(common_);
}

void system::set_level(const std::string &path)
{
    background_.set_level(path);
}

void system::add_sprite(sprite sp, vec2 pos,
                        ::sprite::orientation orient)
{
    sprite_.array.add(
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

}
