/* Copyright 2013-2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include <SDL.h>
#include <SDL_image.h>
#include <cstdio>
#include <cstdlib>
#include "defs.hpp"
#include "opengl.hpp"
#include "test.hpp"

namespace core {

static const int WIDTH = 1280;
static const int HEIGHT = 720;

SDL_Window *window;
SDL_GLContext context;

__attribute__((noreturn))
void die(const char *reason)
{
    std::fprintf(stderr, "Error: %s\n", reason);
    SDL_Quit();
    std::exit(1);
}

__attribute__((noreturn))
void die_alloc()
{
    die("Out of memory");
}

void check_sdl_error(const char *file, int line)
{
    const char *error = SDL_GetError();
    if (*error) {
        std::fprintf(stderr, "SDL error: %s:%d: %s\n", file, line, error);
        SDL_ClearError();
    }
}

__attribute__((noreturn))
void die_sdl(const char *file, int line, const char *msg)
{
    const char *error = SDL_GetError();
    if (*error) {
        std::fprintf(stderr, "Error: %s:%d: %s: %s\n",
                     file, line, msg, error);
    } else {
        std::fprintf(stderr, "Error: %s:%d: %s\n",
                     file, line, msg);
    }
    SDL_Quit();
    std::exit(1);
}

struct gl_error {
    unsigned short code;
    const char name[30];
};

static const gl_error GLERRORS[] = {
    { 0x0500, "INVALID_ENUM" },
    { 0x0501, "INVALID_VALUE" },
    { 0x0502, "INVALID_OPERATION" },
    { 0x0503, "STACK_OVERFLOW" },
    { 0x0504, "STACK_UNDERFLOW" },
    { 0x0505, "OUT_OF_MEMORY" },
    { 0x0506, "INVALID_FRAMEBUFFER_OPERATION" },
    { 0x8031, "TABLE_TOO_LARGE" },
};

void check_gl_error(const char *file, int line)
{
    GLenum error;
    while ((error = glGetError())) {
        int i, n = sizeof(GLERRORS) / sizeof(*GLERRORS);
        for (i = 0; i < n; i++) {
            if (error == GLERRORS[i].code) {
                std::fprintf(stderr, "OpenGL error: %s:%d: GL_%s\n",
                             file, line, GLERRORS[i].name);
                break;
            }
        }
        if (i == n) {
            std::fprintf(stderr, "OpenGL error: %s:%d: Unknown (0x%04x)\n",
                         file, line, (unsigned) error);
        }
    }
}

void delay(int msec)
{
    SDL_Delay(msec);
}

void swap_window()
{
    SDL_GL_SwapWindow(window);
}

void init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        die("Unable to initialize SDL");

    int flags = IMG_INIT_PNG;
    int result = IMG_Init(flags);
    check_sdl_error(HERE);
    if ((result & flags) != flags)
        die("Unable to initialize SDL_image");

    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    // SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow(
        "The Oubliette Within",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_OPENGL);
    check_sdl_error(HERE);
    if (!window)
        die("Unable to create window");

    context = SDL_GL_CreateContext(window);
    check_sdl_error(HERE);
    if (!context)
        die("Unable to create OpenGL context");

    GLenum glewstatus = glewInit();
    if (glewstatus != GLEW_OK) {
        std::fprintf(stderr, "GLEW error: %s\n",
                     glewGetErrorString(glewstatus));
        die("Could not initialize OpenGL.");
    }

    if (!GLEW_VERSION_2_1)
        die("OpenGL 2.1 is missing");
}

void term()
{
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(core::window);
    SDL_Quit();
}

}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    core::init();
    test::test_sprite_sheet(core::WIDTH, core::HEIGHT);
    core::term();

    return 0;
}
