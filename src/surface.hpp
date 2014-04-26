/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_SURFACE_HPP
#define LD_SURFACE_HPP

#include <SDL.h>

namespace sdl {

/// SDL surface with RAII.
struct surface {
    SDL_Surface *surfptr;

    surface();
    surface(const surface &other) = delete;
    surface(surface &&other);
    ~surface();
    surface &operator=(const surface &other) = delete;
    surface &operator=(surface &&other);
};

}

#endif
