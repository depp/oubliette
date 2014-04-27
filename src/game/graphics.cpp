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

program_data::program_data()
    : sprite("sprite", "sprite"),
      tv("tv", "tv")
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

void sprite_data::draw(const program_data &prog)
{
    glUseProgram(prog.sprite.prog());
    glEnableVertexAttribArray(prog.sprite->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sheet.texture());

    glUniform2f(
        prog.sprite->u_vertoff,
        -1.0f,
        -1.0f);
    glUniform2f(
        prog.sprite->u_vertscale,
        2.0 / core::PWIDTH,
        2.0 / core::PHEIGHT);
    glUniform2fv(prog.sprite->u_texscale, 1, sheet.texscale());
    glUniform1i(prog.sprite->u_texture, 0);
    array.set_attrib(prog.sprite->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(prog.sprite->a_vert);
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

void background_data::draw(const program_data &prog)
{
    if (bgtex.tex == 0)
        return;

    glUseProgram(prog.sprite.prog());
    glEnableVertexAttribArray(prog.sprite->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bgtex.tex);

    glUniform2f(
        prog.sprite->u_vertoff,
        -1.0f,
        -1.0f);
    glUniform2f(
        prog.sprite->u_vertscale,
        2.0 / core::PWIDTH,
        2.0 / core::PHEIGHT);
    glUniform2fv(prog.sprite->u_texscale, 1, bgtex.scale);
    glUniform1i(prog.sprite->u_texture, 0);
    array.set_attrib(prog.sprite->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(prog.sprite->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);
}

void background_data::set_level(const std::string &path)
{
    std::string fullpath("level/");
    fullpath += path;
    fullpath += ".png";

    bgtex = image::texture::load(fullpath);
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

void scale_data::end(const program_data &prog)
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

    glUseProgram(prog.tv.prog());
    glEnableVertexAttribArray(prog.tv->a_vert);
    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texpattern.tex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, texbanding.tex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texnoise.tex);

    glUniform1i(prog.tv->u_picture, 0);
    glUniform1i(prog.tv->u_pattern, 1);
    glUniform1i(prog.tv->u_banding, 2);
    glUniform1i(prog.tv->u_noise, 3);
    glUniform4fv(prog.tv->u_noiseoffset, 1, offsets);
    glUniform2fv(prog.tv->u_texscale, 1, scale);
    array.set_attrib(prog.tv->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(prog.tv->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);
}

// ======================================================================

system::system()
{ }

system::~system()
{ }

void system::begin()
{
    sprite_.clear();
    background_.clear();
}

void system::end()
{
    sprite_.upload();
    background_.upload();
}

void system::draw()
{
    scale_.begin();
    background_.draw(prog_);
    sprite_.draw(prog_);
    scale_.end(prog_);
}

void system::set_level(const std::string &path)
{
    background_.set_level(path);
}

void system::add_sprite(sprite sp, game::vec2 pos,
                        ::sprite::orientation orient)
{
    sprite_.array.add(
        sprite_.sheet.get(static_cast<int>(sp)),
        (int)std::floor(pos.x),
        (int)std::floor(pos.y),
        orient);
}

}
