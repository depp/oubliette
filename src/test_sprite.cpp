/* Copyright 2013-2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "shader.hpp"
#include "sprite.hpp"
#include "test.hpp"

namespace test {

void test_sprite(int width, int height)
{
    using sprite::orientation;
    shader::program<shader::sprite> prog("sprite", "spritedummy");
    sprite::rect rect = { 0, 0, 128, 64 };
    sprite::array arr;

    arr.add(rect, 16 + 144*0, 16 + 144*0);
    arr.add(rect, 16 + 144*0, 16 + 144*1, orientation::NORMAL);
    arr.add(rect, 16 + 144*1, 16 + 144*1, orientation::ROTATE_90);
    arr.add(rect, 16 + 144*2, 16 + 144*1, orientation::ROTATE_180);
    arr.add(rect, 16 + 144*3, 16 + 144*1, orientation::ROTATE_270);
    arr.add(rect, 16 + 144*0, 16 + 144*2, orientation::FLIP_VERTICAL);
    arr.add(rect, 16 + 144*1, 16 + 144*2, orientation::TRANSPOSE_2);
    arr.add(rect, 16 + 144*2, 16 + 144*2, orientation::FLIP_HORIZONTAL);
    arr.add(rect, 16 + 144*3, 16 + 144*2, orientation::TRANSPOSE);

    arr.upload(GL_STATIC_DRAW);

    core::check_gl_error(HERE);

    glClearColor(0.5f, 0.6f, 0.7f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(prog.prog());
    glEnableVertexAttribArray(prog->a_vert);

    glUniform2f(prog->u_vertoff, -1.0f, -1.0f);
    glUniform2f(prog->u_vertscale, 2.0 / width, 2.0 / height);
    glUniform2f(prog->u_texscale, 1.0 / 128, 1.0 / 64);
    arr.set_attrib(prog->a_vert);

    glDrawArrays(GL_TRIANGLES, 0, arr.size());

    glDisableVertexAttribArray(prog->a_vert);
    glUseProgram(0);

    core::check_gl_error(HERE);

    core::swap_window();
    core::delay(10000);
}

}
