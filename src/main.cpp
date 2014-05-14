/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "defs.hpp"
#include "opengl.hpp"
#include "rand.hpp"
#include "game/state.hpp"

#if defined _WIN32
#include <Windows.h>
#undef DELETE
#else
#include <unistd.h>
#endif
#if defined __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace core {

SDL_Window *window;
SDL_GLContext context;

NORETURN
void die(const char *reason)
{
    std::fprintf(stderr, "Error: %s\n", reason);
    SDL_Quit();
    std::exit(1);
}

NORETURN
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

NORETURN
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

static const char ERR_DIR[] = "Could not find data directory.";
static const char DEFAULT_DIR[] = "Data";

static void init_path(const char *data_dir)
{
#if defined _WIN32
    if (!data_dir)
        data_dir = DEFAULT_DIR;
    BOOL r = SetCurrentDirectoryA(data_dir);
    if (!r)
        die(ERR_DIR);
#else
    int r;
#if defined __APPLE__
    if (!data_dir)
    {
        CFBundleRef bundle = CFBundleGetMainBundle();
        if (!bundle)
            die(ERR_DIR);
        CFURLRef url = CFBundleCopyBundleURL(bundle);
        if (!url)
            die(ERR_DIR);
        char buf[256];
        r = CFURLGetFileSystemRepresentation(
            url, TRUE, (UInt8 *)buf, sizeof(buf) - sizeof(DEFAULT_DIR));
        if (!r)
            die(ERR_DIR);
        char *p = std::strrchr(buf, '/');
        if (p == nullptr)
            die(ERR_DIR);
        std::strcpy(p + 1, DEFAULT_DIR);
        r = chdir(buf);
        if (r)
            die(ERR_DIR);
        return;
    }
#else
    if (!data_dir)
        data_dir = DEFAULT_DIR;
#endif
    r = chdir(data_dir);
    if (r)
        die(ERR_DIR);
#endif
}

void init()
{
    int result, flags;

    flags = SDL_INIT_VIDEO | SDL_INIT_TIMER |
        SDL_INIT_AUDIO | SDL_INIT_EVENTS;
    result = SDL_Init(flags);
    if (result < 0)
        die("Unable to initialize SDL");

    flags = IMG_INIT_PNG;
    result = IMG_Init(flags);
    check_sdl_error(HERE);
    if ((result & flags) != flags)
        die("Unable to initialize SDL_image");

    flags = MIX_INIT_OGG;
    result = Mix_Init(flags);
    if ((result & flags) != flags) {
        std::puts("Unable to initialize SDL_mixer");
        return;
    }

    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    // SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow(
        "The Oubliette Within",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        IWIDTH,
        IHEIGHT,
        SDL_WINDOW_OPENGL);
    check_sdl_error(HERE);
    if (!window)
        die("Unable to create window");

    context = SDL_GL_CreateContext(window);
    check_sdl_error(HERE);
    if (!context)
        die("Unable to create OpenGL context");

#if defined USE_GLEW
    GLenum glewstatus = glewInit();
    if (glewstatus != GLEW_OK) {
        std::fprintf(stderr, "GLEW error: %s\n",
                     glewGetErrorString(glewstatus));
        die("Could not initialize OpenGL.");
    }

    if (!GLEW_VERSION_2_1)
        die("OpenGL 2.1 is missing");
#endif

    result = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    if (result != 0)
        std::printf("Could not start audio: %s\n", Mix_GetError());

    rng::global.init();
}

void term()
{
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(core::window);
    SDL_Quit();
}

}

using game::key;
static bool decode_key(int scancode, key *k)
{
    switch (scancode) {
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_UP:
        *k = key::UP;
        return true;

    case SDL_SCANCODE_A:
    case SDL_SCANCODE_LEFT:
        *k = key::LEFT;
        return true;

    case SDL_SCANCODE_S:
    case SDL_SCANCODE_DOWN:
        *k = key::DOWN;
        return true;

    case SDL_SCANCODE_D:
    case SDL_SCANCODE_RIGHT:
        *k = key::RIGHT;
        return true;

    case SDL_SCANCODE_SPACE:
    case SDL_SCANCODE_RETURN:
        *k = key::SELECT;
        return true;

    case SDL_SCANCODE_TAB:
    case SDL_SCANCODE_PAGEDOWN:
        *k = key::NEXT;
        return true;

    case SDL_SCANCODE_PAGEUP:
        *k = key::PREV;
        return true;

    case SDL_SCANCODE_BACKSPACE:
    case SDL_SCANCODE_DELETE:
        *k = key::DELETE;
        return true;

    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT:
        *k = key::SHIFT;
        return true;

    default:
        return false;
    }
}

int main(int argc, char *argv[])
{
    const char *start_level = "difficulty";
    const char *data_dir = nullptr;
    bool edit_mode = false;
    int i = 1;
    while (i < argc) {
        const char *a = argv[i];
        std::size_t len = std::strlen(a);
        if (a[0] != '-') {
            start_level = a;
            i++;
        } else if (!std::strcmp(a, "-d") || !std::strcmp(a, "--dir")) {
            i++;
            if (i >= argc) {
                std::fprintf(stderr, "Warning: --dir/-d needs an argument\n");
                continue;
            }
            data_dir = argv[i];
            i++;
        } else if (!std::strcmp(a, "--edit") || !std::strcmp(a, "-e")) {
            edit_mode = true;
            i++;
        } else if (len >= 4 && !std::memcmp(a, "-psn", 4)) {
            i++;
        } else if (len >= 3 && !std::memcmp(a, "-NS", 3)) {
            i += 2;
        } else {
            std::fprintf(stderr, "Warning: unknown argument: %s\n", a);
            i++;
        }
    }

    const unsigned MIN_TICKS1 = 1000 / core::MAXFPS;
    const unsigned MIN_TICKS = MIN_TICKS1 > 0 ? MIN_TICKS1 : 1;

    core::init();
    core::init_path(data_dir);

    {
        bool do_quit = false;
        unsigned last_frame = SDL_GetTicks();
        game::state gstate(edit_mode);
        gstate.set_level(start_level);
        while (!do_quit) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                switch (e.common.type) {
                case SDL_QUIT:
                    do_quit = true;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    gstate.mouse_click(
                        e.button.x / core::SCALE,
                        (core::IHEIGHT - 1 - e.button.y) / core::SCALE,
                        e.common.type == SDL_MOUSEBUTTONDOWN ?
                        e.button.button : -1);
                    break;

                case SDL_MOUSEMOTION:
                    gstate.mouse_move(
                        e.motion.x / core::SCALE,
                        (core::IHEIGHT - 1 - e.motion.y) / core::SCALE);
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    key k;
                    if (decode_key(e.key.keysym.scancode, &k))
                        gstate.event_key(k, e.common.type == SDL_KEYDOWN);
                    break;
                }
            }

            gstate.draw(SDL_GetTicks());
            core::swap_window();

            unsigned now = SDL_GetTicks();
            unsigned delta = now - last_frame;
            if (delta < MIN_TICKS) {
                SDL_Delay(MIN_TICKS - delta);
                now = SDL_GetTicks();
            }

            last_frame = now;
        }
    }

    core::term();

    return 0;
}
