/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_OPENGL_HPP
#define LD_OPENGL_HPP

#if defined __APPLE__
#include <OpenGL/gl.h>
#else
#define USE_GLEW
#include <GL/glew.h>
#endif

#endif
