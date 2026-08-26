#pragma once
#include <cassert>
#include <cmath>
#include <type_traits>

// 1. Generic absolute value helper
template <typename T>
[[nodiscard]] constexpr T cabs(T x) {
    return x < T(0) ? -x : x;
}

// 2. Precision-aware Pi constant templates
template <typename T>
constexpr T PI_v = T(3.141592653589793238462643383279502884L);

constexpr double PI = PI_v<double>;
constexpr float PIf = PI_v<float>;

// 3. Templated 3D Vector
template <typename T>
struct basic_vec3 {
    using value_type = T;

    T x{0}, y{0}, z{0};

    constexpr basic_vec3& operator+=(basic_vec3 v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    constexpr basic_vec3& operator-=(basic_vec3 v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    constexpr basic_vec3& operator*=(T s) {
        x *= s; y *= s; z *= s;
        return *this;
    }
    constexpr basic_vec3& operator/=(T s) {
        return *this *= (T(1) / s);
    }
};

// 4. Non-member operators
template <typename T>
constexpr basic_vec3<T> operator+(basic_vec3<T> a, basic_vec3<T> b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

template <typename T>
constexpr basic_vec3<T> operator-(basic_vec3<T> a, basic_vec3<T> b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

template <typename T>
constexpr basic_vec3<T> operator-(basic_vec3<T> a) {
    return {-a.x, -a.y, -a.z};
}

template <typename T>
constexpr basic_vec3<T> operator*(basic_vec3<T> v, T s) {
    return {v.x * s, v.y * s, v.z * s};
}

template <typename T>
constexpr basic_vec3<T> operator*(T s, basic_vec3<T> v) {
    return v * s;
}

template <typename T>
constexpr basic_vec3<T> operator/(basic_vec3<T> v, T s) {
    return v * (T(1) / s);
}

// 5. Vector Operations
template <typename T>
[[nodiscard]] constexpr T dot(basic_vec3<T> a, basic_vec3<T> b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
[[nodiscard]] constexpr basic_vec3<T> cross(basic_vec3<T> a, basic_vec3<T> b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

template <typename T>
[[nodiscard]] constexpr T norm_squared(basic_vec3<T> v) {
    return dot(v, v);
}

// In C++20, std::sqrt is fully constexpr for standard floating-point types!
template <typename T>
[[nodiscard]] constexpr T norm(basic_vec3<T> v) {
    return std::sqrt(norm_squared(v));
}

template <typename T>
[[nodiscard]] constexpr basic_vec3<T> normalized(basic_vec3<T> v) {
    assert(norm_squared(v) > T(0) && "normalizing zero vector");
    return v / norm(v);
}

template <typename T>
[[nodiscard]] constexpr basic_vec3<T> project_onto(basic_vec3<T> a, basic_vec3<T> b) {
    assert(dot(b, b) > T(0) && "projection onto zero vector");
    return b * (dot(a, b) / dot(b, b));
}

// 6. Tolerance Comparison (Defaults change based on type)
template <typename T>
[[nodiscard]] constexpr bool approx_equal(T a, T b, 
    T eps_abs = std::is_same_v<T, float> ? T(1e-5) : T(1e-9), 
    T eps_rel = std::is_same_v<T, float> ? T(1e-5) : T(1e-9)) 
{
    const T diff = cabs(a - b);
    const T largest = cabs(a) > cabs(b) ? cabs(a) : cabs(b);
    return diff <= eps_abs || diff <= eps_rel * largest;
}

template <typename T>
[[nodiscard]] constexpr bool approx_equal(basic_vec3<T> a, basic_vec3<T> b, 
    T eps_abs = std::is_same_v<T, float> ? T(1e-5) : T(1e-9), 
    T eps_rel = std::is_same_v<T, float> ? T(1e-5) : T(1e-9)) 
{
    return approx_equal(a.x, b.x, eps_abs, eps_rel) && 
           approx_equal(a.y, b.y, eps_abs, eps_rel) && 
           approx_equal(a.z, b.z, eps_abs, eps_rel);
}

// 7. Clean Public Aliases
using vec3  = basic_vec3<double>; // Keeps backward compatibility with your old code
using vec3d = basic_vec3<double>;
using vec3f = basic_vec3<float>;
