/* Copyright 2014 Dietrich Epp.
   This file is part of LDBase.  LDBase is licensed under the terms
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

}

#endif
