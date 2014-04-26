/* Copyright 2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#include "surface.hpp"

namespace sdl {

surface::surface()
    : surfptr(nullptr)
{
}

surface::surface(surface &&other)
    : surfptr(nullptr)
{
    surfptr = other.surfptr;
    other.surfptr = nullptr;
}

surface::~surface()
{
    if (surfptr)
        SDL_FreeSurface(surfptr);
}

surface &surface::operator=(surface &&other)
{
    if (this == &other)
        return *this;
    if (surfptr)
        SDL_FreeSurface(surfptr);
    surfptr = other.surfptr;
    other.surfptr = nullptr;
    return *this;
}

}
