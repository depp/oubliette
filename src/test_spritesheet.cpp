/* Copyright 2013-2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "shader.hpp"
#include "sprite.hpp"
#include "test.hpp"

namespace test {

void test_sprite_sheet(int width, int height)
{
    static sprite::sprite SPRITES[] = {
        { "arrow", 0, 0, 64, 64 },
        { "many", 0, 0, 64, 32 },
        { "many", 0, 32, 32, 64 },
        { "many", 32, 32, 32, 64 },
        { "many", 0, 96, 32, 32 },
        { "many", 32, 96, 32, 32 },
        { "alpha", 0, 0, 64, 64 },
        { nullptr, 0, 0, 0, 0 }
    };

    sprite::sheet sheet("test", SPRITES);

    shader::program<shader::sprite> prog("sprite", "sprite");

    sprite::array arr;

    arr.add(sheet.get(0), 16 + 144*0, 16 + 144*0);
    arr.add(sheet.get(1), 16 + 144*1, 16 + 144*0);
    arr.add(sheet.get(2), 16 + 144*2, 16 + 144*0);
    arr.add(sheet.get(3), 16 + 144*0, 16 + 144*1);
    arr.add(sheet.get(4), 16 + 144*1, 16 + 144*1);
    arr.add(sheet.get(5), 16 + 144*2, 16 + 144*1);
    arr.add(sheet.get(6), 16 + 144*0, 16 + 144*2);

    arr.upload(GL_STATIC_DRAW);

    core::check_gl_error(HERE);

    glClearColor(0.5f, 0.6f, 0.7f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(prog.prog());
    glEnableVertexAttribArray(prog->a_vert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sheet.texture());

    glUniform2f(prog->u_vertoff, -1.0f, -1.0f);
    glUniform2f(prog->u_vertscale, 2.0 / width, 2.0 / height);
    glUniform2fv(prog->u_texscale, 1, sheet.texscale());
    arr.set_attrib(prog->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, arr.size());

    glDisableVertexAttribArray(prog->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);

    core::swap_window();
    core::delay(10000);
}

}
