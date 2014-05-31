/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_VEC_HPP
#define LD_GAME_VEC_HPP
#include <cmath>

/// Two-dimensional vector.
struct fvec {
    float x, y;

    fvec() { }
    fvec(float x, float y) : x(x), y(y) { }
    static fvec zero() { return fvec(0, 0); }

    /// Get the magnitude of the vector, squared.
    float mag2() const;
    /// Get the magnitude of the vector.
    float mag() const;
    /// Get the dot product of two vectors.
    static float dot(fvec u, fvec v);
    /// Get the distance between two vectors, squared.
    static float dist2(fvec u, fvec v);
    /// Get the distance between two vectors.
    static float dist(fvec u, fvec v);
};

inline fvec operator+(fvec u, fvec v) { return fvec(u.x + v.x, u.y + v.y); }
inline fvec &operator+=(fvec &u, fvec v) { u.x += v.x; u.y += v.y; return u; }
inline fvec operator-(fvec u, fvec v) { return fvec(u.x - v.x, u.y - v.y); }
inline fvec &operator-=(fvec &u, fvec v) { u.x -= v.x; u.y -= v.y; return u; }
inline fvec operator*(float a, fvec v) { return fvec(a * v.x, a * v.y); }
inline fvec operator*(fvec v, float a) { return fvec(a * v.x, a * v.y); }
inline fvec &operator*=(fvec &v, float a) { v.x *= a; v.y *= a; return v; }

inline float fvec::mag2() const { return x * x + y * y; }
inline float fvec::mag() const { return std::sqrt(mag()); }
inline float fvec::dot(fvec u, fvec v) { return u.x * v.x + u.y * v.y; }
inline float fvec::dist2(fvec u, fvec v) { return (u - v).mag2(); }
inline float fvec::dist(fvec u, fvec v) { return std::sqrt(dist2(u, v)); }

// Floating-point rectangle.
struct frect {
    float x0, y0, x1, y1;

    frect() { }
    frect(float x0, float y0, float x1, float y1)
        : x0(x0), y0(y0), x1(x1), y1(y1)
    { }

    static bool test_intersect(const frect &a, const frect &b);
    fvec center() const;
    frect offset(fvec v) const;
    frect expand(float amt) const;
    frect expand(float horiz, float vert) const;
    frect expand(const frect &r) const;
    fvec nearest(fvec v) const;
};

/// Integer vector.
struct ivec {
    int x, y;
    ivec() { };
    ivec(int x, int y) : x(x), y(y) { }
    explicit ivec(fvec v)
        : x((int)std::floor(v.x)),
          y((int)std::floor(v.y))
    { }

    static ivec zero() { return ivec(0, 0); }
    explicit operator fvec() const { return fvec(x, y); }
};

inline ivec operator+(ivec u, ivec v) { return ivec(u.x + v.x, u.y + v.y); }
inline ivec &operator+=(ivec &u, ivec v) { u.x += v.x; u.y += v.y; return u; }
inline ivec operator-(ivec u, ivec v) { return ivec(u.x - v.x, u.y - v.y); }
inline ivec &operator-=(ivec &u, ivec v) { u.x -= v.x; u.y -= v.y; return u; }
inline ivec operator*(int a, ivec v) { return ivec(a * v.x, a * v.y); }
inline ivec operator*(ivec v, int a) { return ivec(a * v.x, a * v.y); }
inline ivec &operator*=(ivec &v, int a) { v.x *= a; v.y *= a; return v; }

/// Integer rectangle.
struct irect {
    int x0, y0, x1, y1;

    irect() { }
    irect(int x0, int y0, int x1, int y1)
        : x0(x0), y0(y0), x1(x1), y1(y1)
    { }

    static irect centered(int w, int h);
    static bool test_intersect(const irect &a, const irect &b);
    ivec center() const;
    irect offset(ivec v) const;
    irect expand(int amt) const;
    irect expand(int horiz, int vert) const;
    irect expand(const irect &r) const;
    bool contains(ivec v) const;
};

#endif
