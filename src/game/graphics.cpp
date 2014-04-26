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
#include <cstdio>
namespace game {

static const sprite::sprite SPRITES[] = {
    { "player", 0, 0, 16, 24 },
    { nullptr, 0, 0, 0, 0 }
};

struct graphics_system::data {
    shader::program<shader::sprite> prog_sprite;
    sprite::sheet sheet;
    sprite::array array_sprite;

    data();

    void update(state &s, int reltime);
    void draw();
};

graphics_system::data::data()
    : prog_sprite("sprite", "sprite"),
      sheet("sprite", SPRITES)
{
}

void graphics_system::data::update(state &s, int reltime)
{
    (void)reltime;
    array_sprite.clear();
    scalar timefrac = reltime * (scalar) (1.0 / defs::FRAMETIME);

    for (auto i = s.physics.objects.begin(),
             e = s.physics.objects.end(); i != e; i++) {
        auto pos = i->lastpos + timefrac * (i->pos - i->lastpos);
        array_sprite.add(sheet.get(0), pos.x - 6, pos.y - 12);
    }

    array_sprite.upload(GL_DYNAMIC_DRAW);
    core::check_gl_error(HERE);
}

void graphics_system::data::draw()
{
    // Clear screen

    glClearColor(0.125f, 0.125f, 0.125f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw sprites

    glUseProgram(prog_sprite.prog());
    glEnableVertexAttribArray(prog_sprite->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sheet.texture());

    glUniform2f(
        prog_sprite->u_vertoff,
        -1.0f,
        -1.0f);
    glUniform2f(
        prog_sprite->u_vertscale,
        2.0 / core::WIDTH,
        2.0 / core::HEIGHT);
    glUniform2fv(prog_sprite->u_texscale, 1, sheet.texscale());
    array_sprite.set_attrib(prog_sprite->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, array_sprite.size());

    glDisableVertexAttribArray(prog_sprite->a_vert);
    glUseProgram(0);
}

graphics_system::graphics_system()
{
}

graphics_system::~graphics_system()
{
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
    if (!data_)
        core::die("graphics not initialized");
    data_->update(s, reltime);
    data_->draw();
}

}
