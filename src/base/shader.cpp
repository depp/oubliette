/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "defs.hpp"
#include "file.hpp"
#include "shader.hpp"
#include <cstdio>
#include <stdexcept>
#include <assert.h>

namespace shader {

/// Load a GLSL shader.  Returns 0 on failure.
GLuint load_shader(const std::string &name, GLenum type)
{
    std::string path("shader/");
    path += name;
    switch (type) {
    case GL_VERTEX_SHADER: path += ".vert.glsl"; break;
    case GL_FRAGMENT_SHADER: path += ".frag.glsl"; break;
    default: assert(0);
    }

    core::check_gl_error(HERE);

    data data;
    if (!data::read(&data, path))
        return 0;

    const char *darr[1];
    GLint larr[1];

    GLuint shader = glCreateShader(type);
    darr[0] = static_cast<const char *>(data.ptr());
    larr[0] = data.size();
    glShaderSource(shader, 1, darr, larr);
    glCompileShader(shader);
    GLint flag;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &flag);
    if (flag) {
        core::check_gl_error(HERE);
        return shader;
    }

    GLint loglen;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &loglen);
    std::fprintf(stderr, "%s: compilation failed\n", path.c_str());
    if (loglen > 0) {
        char *log = new char[loglen];
        glGetShaderInfoLog(shader, loglen, NULL, log);
        std::fputs(log, stderr);
        std::fputc('\n', stderr);
        delete[] log;
    }
    glDeleteShader(shader);
    return 0;
}

/// Link a shader program.  Returns false on failure.
bool LinkProgram(GLuint prog, std::string &name)
{
    GLint flag, loglen;
    char *log;

    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &flag);
    if (flag)
        return true;

    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &loglen);
    std::fprintf(stderr, "%s: linking failed", name.c_str());
    if (loglen > 0) {
        log = new char[loglen];
        glGetProgramInfoLog(prog, loglen, NULL, log);
        std::fputs(log, stderr);
        std::fputc('\n', stderr);
        delete[] log;
    }
    return false;
}

/// Load the indexes of shader uniforms into an object.
void get_uniforms(GLuint prog, void *object, const field *uniforms)
{
    for (int i = 0; uniforms[i].name; i++) {
        GLint *ptr = reinterpret_cast<GLint *>(
            static_cast<char *>(object) + uniforms[i].offset);
        *ptr = glGetUniformLocation(prog, uniforms[i].name);
    }
}

/// Load the indexes of shader attributes into an object.
void get_attributes(GLuint prog, void *object, const field *attributes)
{
    for (int i = 0; attributes[i].name; i++) {
        GLint *ptr = reinterpret_cast<GLint *>(
            static_cast<char *>(object) + attributes[i].offset);
        *ptr = glGetAttribLocation(prog, attributes[i].name);
    }
}

GLuint load_program(const std::string &vertexshader,
                    const std::string &fragmentshader,
                    const field *uniforms,
                    const field *attributes,
                    void *object)
{
    std::string name = vertexshader + ", " + fragmentshader;
    GLuint vertex = load_shader(vertexshader, GL_VERTEX_SHADER);
    if (vertex == 0) {
        return 0;
    }
    GLuint fragment = load_shader(fragmentshader, GL_FRAGMENT_SHADER);
    if (fragment == 0) {
        glDeleteShader(vertex);
        return 0;
    }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertex);
    glAttachShader(prog, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (!LinkProgram(prog, name)) {
        glDeleteProgram(prog);
        return 0;
    }
    get_uniforms(prog, object, uniforms);
    get_attributes(prog, object, attributes);
    return prog;
}

#define FIELD(n) { #n, offsetof(TYPE, n) }

#define TYPE plain
const field plain::UNIFORMS[] = {
    FIELD(u_vertxform),
    FIELD(u_color),
    { nullptr, 0 }
};

const field plain::ATTRIBUTES[] = {
    FIELD(a_vert),
    { nullptr, 0 }
};
#undef TYPE

#define TYPE sprite
const field sprite::UNIFORMS[] = {
    FIELD(u_vertxform),
    FIELD(u_texscale),
    FIELD(u_texture),
    { nullptr, 0 }
};

const field sprite::ATTRIBUTES[] = {
    FIELD(a_vert),
    { nullptr, 0 }
};
#undef TYPE

#define TYPE text
const field text::UNIFORMS[] = {
    FIELD(u_vertxform),
    FIELD(u_texscale),
    FIELD(u_texture),
    FIELD(u_color),
    { nullptr, 0 }
};

const field text::ATTRIBUTES[] = {
    FIELD(a_vert),
    { nullptr, 0 }
};
#undef TYPE

#define TYPE tv
const field tv::UNIFORMS[] = {
    FIELD(u_picture),
    FIELD(u_pattern),
    FIELD(u_banding),
    FIELD(u_noise),
    FIELD(u_noiseoffset),
    FIELD(u_texscale),
    FIELD(u_color),
    { nullptr, 0 }
};

const field tv::ATTRIBUTES[] = {
    FIELD(a_vert),
    { nullptr, 0 }
};
#undef TYPE

}
