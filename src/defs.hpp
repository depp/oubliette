/* Copyright 2013-2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_DEFS_HPP
#define LD_DEFS_HPP

#define HERE __FILE__, __LINE__

namespace core {

/// Cause the application to exit with the given human-readable reason.
__attribute__((noreturn))
void die(const char *reason);

/// Cause the application to exit because an allocation failed.
__attribute__((noreturn))
void die_alloc();

/// Cause the application to exit because of SDL errors.
__attribute__((noreturn))
void die_sdl(const char *file, int line, const char *msg);

/// Check for SDL errors and log them.
void check_sdl_error(const char *file, int line);

/// Check for OpenGL errors and log them.
void check_gl_error(const char *file, int line);

/// Pause for the given number of milliseconds.
void delay(int msec);

/// Swap the main window buffers.
void swap_window();

}

#endif
