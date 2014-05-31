/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_RAND_HPP
#define LD_RAND_HPP

struct rng {
    static rng global;

    unsigned x, y, z, w;

    unsigned next()
    {
        unsigned xx = x;
        unsigned tt = xx ^ (xx << 11);
        x = y;
        y = z;
        z = w;
        unsigned ww = w;
        w = ww ^ (ww >> 19) ^ (tt ^ (tt >> 8));
        return ww;
    }

    void init();
};

#endif
