/* Copyright 2014 Dietrich Epp.
   This file is part of Oubliette.  Oubliette is licensed under the terms
   of the 2-clause BSD license.  For more information, see LICENSE.txt. */
#ifndef LD_GAME_VEC_HPP
#define LD_GAME_VEC_HPP
#include <cmath>
namespace game {

typedef float scalar;

/// Two-dimensional vector.
struct vec2 {
    scalar x, y;

    vec2() { }
    vec2(scalar x, scalar y) : x(x), y(y) { }
    static vec2 zero() { return vec2(0, 0); }

    /// Get the magnitude of the vector, squared.
    scalar mag2() const;
    /// Get the magnitude of the vector.
    scalar mag() const;
    /// Get the dot product of two vectors.
    static scalar dot(vec2 u, vec2 v);
    /// Get the distance between two vectors, squared.
    static scalar dist2(vec2 u, vec2 v);
    /// Get the distance between two vectors.
    static scalar dist(vec2 u, vec2 v);
};

inline vec2 operator+(vec2 u, vec2 v) { return vec2(u.x + v.x, u.y + v.y); }
inline vec2 &operator+=(vec2 &u, vec2 v) { u.x += v.x; u.y += v.y; return u; }
inline vec2 operator-(vec2 u, vec2 v) { return vec2(u.x - v.x, u.y - v.y); }
inline vec2 &operator-=(vec2 &u, vec2 v) { u.x -= v.x; u.y -= v.y; return u; }
inline vec2 operator*(scalar a, vec2 v) { return vec2(a * v.x, a * v.y); }
inline vec2 operator*(vec2 v, scalar a) { return vec2(a * v.x, a * v.y); }
inline vec2 &operator*=(vec2 &v, scalar a) { v.x *= a; v.y *= a; return v; }

inline scalar vec2::mag2() const { return x * x + y * y; }
inline scalar vec2::mag() const { return std::sqrt(mag()); }
inline scalar vec2::dot(vec2 u, vec2 v) { return u.x * v.x + u.y * v.y; }
inline scalar vec2::dist2(vec2 u, vec2 v) { return (u - v).mag2(); }
inline scalar vec2::dist(vec2 u, vec2 v) { return std::sqrt(dist2(u, v)); }

// Floating-point rectangle.
struct rect {
    vec2 min, max;

    rect() { }
    rect(vec2 min, vec2 max)
        : min(min), max(max)
    { }
    rect(float x0, float y0, float x1, float y1)
        : min(x0, y0), max(x1, y1)
    { }

    static bool test_intersect(const rect &a, const rect &b);
    rect offset(vec2 v) const;
    vec2 nearest(vec2 v) const;
};

/// Integer vector.
struct ivec {
    int x, y;
    ivec() { };
    ivec(int x, int y) : x(x), y(y) { }
    explicit ivec(vec2 v)
        : x((int)std::floor(v.x)),
          y((int)std::floor(v.y))
    { }
};

/// Integer rectangle.
struct irect {
    int x0, y0, x1, y1;

    irect() { }
    irect(int x0, int y0, int x1, int y1)
        : x0(x0), y0(y0), x1(x1), y1(y1)
    { }

    irect offset(int x, int y)
    {
        return irect(x0 + x, y0 + y, x1 + x, y1 + y);
    }

    irect offset(vec2 v)
    {
        return offset(std::floor(v.x), std::floor(v.y));
    }

    irect expand(int amt)
    {
        return irect(x0 - amt, y0 - amt, x1 + amt, y1 + amt);
    }

    bool contains(vec2 pt) const
    {
        int x = (int)std::floor(pt.x), y = (int)std::floor(pt.y);
        return x0 <= x && x < x1 && y0 <= y && y < y1;
    }

    bool contains(ivec v) const
    {
        return contains(v.x, v.y);
    }

    bool contains(int x, int y) const
    {
        return x0 <= x && x < x1 && y0 <= y && y < y1;
    }

    static irect centered(int w, int h)
    {
        return irect(-(w/2), -(h/2), w - (w/2), h - (h/2));
    }
};

}
#endif
