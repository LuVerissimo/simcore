#pragma once

#include <cassert>

constexpr double cabs(double x) { return x < 0.0 ? -x : x; }

constexpr double PI = 3.14159265358979323846;

struct vec3 {
    double x{0.0}, y{0.0}, z{0.0};

    constexpr vec3& operator+=(vec3 v) { x += v.x; y += v.y; z += v.z; return *this; }
    constexpr vec3& operator-=(vec3 v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    constexpr vec3& operator*=(double s) { x *= s; y *= s; z *= s; return *this; }
    constexpr vec3& operator/=(double s) { return *this *= (1.0 / s); }
};


constexpr vec3 operator+(vec3 a, vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
constexpr vec3 operator-(vec3 a, vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
constexpr vec3 operator-(vec3 a)         { return {-a.x, -a.y, -a.z}; }

constexpr vec3 operator*(vec3 v, double s) { return {v.x * s, v.y * s, v.z * s}; }
constexpr vec3 operator*(double s, vec3 v) { return v * s; }
constexpr vec3 operator/(vec3 v, double s) { return v * (1.0 / s); }

[[nodiscard]] constexpr double dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

[[nodiscard]] constexpr vec3 cross(vec3 a, vec3 b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

[[nodiscard]] constexpr double norm_squared(vec3 v) { return dot(v, v); }

// sqrt is not constexpr in C++20 → plain inline. Prefer norm_squared for comparisons.
#include <cmath>
[[nodiscard]] inline double norm(vec3 v) { return std::sqrt(norm_squared(v)); }

// Returns a copy; does not mutate. Mutators would be verbs (normalize()).
[[nodiscard]] inline vec3 normalized(vec3 v) {
    assert(norm_squared(v) > 0.0 && "normalizing zero vector");
    return v / norm(v);
}

// Contract: b must be nonzero. Assert = free in release, loud in debug.
[[nodiscard]] constexpr vec3 project_onto(vec3 a, vec3 b) {
    assert(dot(b, b) > 0.0 && "projection onto zero vector");
    return b * (dot(a, b) / dot(b, b));
}

// Absolute-or-relative tolerance (see formula sheet).
[[nodiscard]] constexpr bool approx_equal(double a, double b,
                                          double eps_abs = 1e-9,
                                          double eps_rel = 1e-9) {
    const double diff = cabs(a - b);
    const double largest = cabs(a) > cabs(b) ? cabs(a) : cabs(b);
    return diff <= eps_abs || diff <= eps_rel * largest;
}

[[nodiscard]] constexpr bool approx_equal(vec3 a, vec3 b,
                                          double eps_abs = 1e-9,
                                          double eps_rel = 1e-9) {
    return approx_equal(a.x, b.x, eps_abs, eps_rel) &&
           approx_equal(a.y, b.y, eps_abs, eps_rel) &&
           approx_equal(a.z, b.z, eps_abs, eps_rel);
}