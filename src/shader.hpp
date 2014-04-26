/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_SHADER_HPP
#define LD_SHADER_HPP
#include "opengl.hpp"
#include <cstddef>
#include <string>

namespace shader {

/// A field in an object which stores program attributes and uniform indexes.
struct field {
    /// The name of the uniform or attribute.
    const char *name;

    /// The offset of the corresponding GLint field in the structure.
    std::size_t offset;
};

/// Load an OpenGL shader program.  Returns 0 on failure.
GLuint load_program(const std::string &vertexshader,
                    const std::string &fragmentshader,
                    const field *uniforms,
                    const field *attributes,
                    void *object);

/// An OpenGL shader program.  The parameter T has uniforms and attributes.
template<class T>
class program {
private:
    GLuint prog_;
    T fields_;

public:
    program(const std::string &vertexshader,
            const std::string &fragmentshader);
    program(const program &) = delete;
    ~program();
    program &operator =(const program &) = delete;

    /// Get program attribute and uniform indexes.
    const T *operator->() const { return &fields_; }
    /// Get the program object.
    GLuint prog() const { return prog_; }
};

template<class T>
program<T>::program(const std::string &vertexshader,
                    const std::string &fragmentshader)
    : prog_(0), fields_()
{
    prog_ = load_program(
        vertexshader, fragmentshader,
        T::UNIFORMS, T::ATTRIBUTES,
        &fields_);
}

template<class T>
program<T>::~program()
{
    glDeleteProgram(prog_);
}

/// Uniforms and attributes for the "plain" shader.
struct plain {
    static const field UNIFORMS[];
    static const field ATTRIBUTES[];

    GLint a_vert;
    GLint u_vertmat;
    GLint u_vertoff;
    GLint u_color;
};

/// Uniforms and attributes for the "sprite" shader.
struct sprite {
    static const field UNIFORMS[];
    static const field ATTRIBUTES[];

    GLint a_vert;
    GLint u_vertoff;
    GLint u_vertscale;
    GLint u_texscale;
    GLint u_texture;
};

}

#endif
