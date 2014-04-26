/* Copyright 2013-2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_PACK_HPP
#define LD_PACK_HPP

#include <stddef.h>
#include <vector>
namespace pack {

/// The size of a rectangle to be packed.
struct size {
    int width;
    int height;
};

/// The location of a packed rectangle.
struct location {
    int x;
    int y;
};

/// A complete packing of a set of rectangles.
struct packing {
    /// The size of the rectangle enclosing the packed rectangles.
    size packsize;

    /// The location of the packed rectangles.
    std::vector<location> locations;
};

/// Efficiently pack a set of rectangles in a larger rectangle.
packing pack(const std::vector<size> &rects);

}

#endif
