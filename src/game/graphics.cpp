/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "graphics.hpp"
#include "state.hpp"
#include "defs.hpp"
#include "../defs.hpp"
#include "../opengl.hpp"
#include "../shader.hpp"
#include "../sprite.hpp"
#include "../image.hpp"
#include <cstdio>
namespace game {
namespace graphics {

// ======================================================================

struct program_system {
    shader::program<shader::sprite> sprite;

    program_system();
};

program_system::program_system()
    : sprite("sprite", "sprite")
{ }

// ======================================================================

static const sprite::sprite SPRITES[] = {
    { "player", 0, 0, 16, 24 },
    { nullptr, 0, 0, 0, 0 }
};

struct sprite_system {
    program_system &prog;
    sprite::sheet sheet;
    sprite::array array;

    sprite_system(program_system &prog);

    void update(state &s, int reltime);
    void draw();
};

sprite_system::sprite_system(program_system &prog)
    : prog(prog),
      sheet("sprite", SPRITES)
{ }

void sprite_system::update(state &s, int reltime)
{
    array.clear();
    scalar timefrac = reltime * (scalar) (1.0 / defs::FRAMETIME);

    for (auto i = s.physics.objects.begin(),
             e = s.physics.objects.end(); i != e; i++) {
        vec2 offset(-6, -12);
        auto pos = i->lastpos + timefrac * (i->pos - i->lastpos) + offset;
        array.add(sheet.get(0), floorf(pos.x), floorf(pos.y));
    }

    array.upload(GL_DYNAMIC_DRAW);
    core::check_gl_error(HERE);
}

void sprite_system::draw()
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
        2.0 / core::WIDTH,
        2.0 / core::HEIGHT);
    glUniform2fv(prog.sprite->u_texscale, 1, sheet.texscale());
    array.set_attrib(prog.sprite->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(prog.sprite->a_vert);
    glUseProgram(0);
}

// ======================================================================

struct background_system {
    program_system &prog;
    image::texture bgtex;
    sprite::array array;

    background_system(program_system &prog);

    void update(state &s, int reltime);
    void draw();

    void set_level(const std::string &path);
};

background_system::background_system(program_system &prog)
    : prog(prog)
{ }

void background_system::update(state &s, int reltime)
{
    (void)&s;
    (void)reltime;

    if (bgtex.tex == 0)
        return;

    array.clear();
    sprite::rect r = {
        0, 0,
        bgtex.iwidth, bgtex.iheight
    };
    array.add(r, 0, 0);
    array.upload(GL_DYNAMIC_DRAW);
    core::check_gl_error(HERE);
}

void background_system::draw()
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
        2.0 / core::WIDTH,
        2.0 / core::HEIGHT);
    glUniform2fv(prog.sprite->u_texscale, 1, bgtex.scale);
    array.set_attrib(prog.sprite->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array.size());

    glDisableVertexAttribArray(prog.sprite->a_vert);
    glUseProgram(0);
}

void background_system::set_level(const std::string &path)
{
    std::string fullpath("level/");
    fullpath += path;
    fullpath += ".png";

    bgtex = image::texture::load(fullpath);
}

// ======================================================================

} // namespace graphics

// ======================================================================

struct graphics_system::data {
    graphics::program_system prog;
    graphics::sprite_system sprite;
    graphics::background_system background;

    data();

    void update(state &s, int reltime);
    void draw();
};

graphics_system::data::data()
    : prog(), sprite(prog), background(prog)
{ }

void graphics_system::data::update(state &s, int reltime)
{
    background.update(s, reltime);
    sprite.update(s, reltime);
}

void graphics_system::data::draw()
{
    glClearColor(0.125f, 0.125f, 0.125f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    background.draw();
    sprite.draw();
}

graphics_system::graphics_system()
{
}

graphics_system::~graphics_system()
{
}

graphics_system::data &graphics_system::getdata()
{
    if (!data_)
        core::die("graphics not initialized");
    return *data_;
}

void graphics_system::init()
{
    data_.reset(new data());
}

void graphics_system::term()
{
    data_.reset(nullptr);
}

void graphics_system::draw(state &s, int reltime)
{
    auto &d = getdata();
    d.update(s, reltime);
    d.draw();
}

void graphics_system::set_level(const std::string &path)
{
    getdata().background.set_level(path);
}

}
