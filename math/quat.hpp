#pragma once
#include "vec3.hpp"
#include <cmath>
#include <cassert>
#include <algorithm>
#include "mat3.hpp"

struct quat {
    double w{1.0}, x{0.0}, y{0.0}, z{0.0};  // default = identity rotation

    quat operator*(double scalar) const {
        return { w * scalar, x * scalar, y * scalar, z * scalar};
    }
    quat operator+(const quat& other) const {
        return { w + other.w, x + other.x, y + other.y, z + other.z};
    }

    quat operator-() const { 
        return { -w, -x, -y, -z};
    }
};

inline double dot_product(const quat& a, const quat& b) {
    return (a.w * b.w) + (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

[[nodiscard]] constexpr bool approx_equal(quat a, quat b, double eps = 1e-9) {
    return cabs(a.w-b.w) < eps && cabs(a.x-b.x) < eps && cabs(a.y-b.y) < eps && cabs(a.z-b.z) < eps;
}

// Expand using ij=k, jk=i, ki=j, ji=-k, kj=-i, ik=-j
[[nodiscard]] constexpr quat operator*(quat a, quat b){
    return quat(
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w 
    );
};

[[nodiscard]] constexpr quat conjugate(quat q) { // (w, -x, -y, -z)
    quat conj = quat(q.w, -q.x, -q.y, -q.z);
    return conj;
}           

[[nodiscard]] constexpr double norm_squared(quat q) {  // w²+x²+y²+z² 
    return (q.w * q.w) + (q.x * q.x) + (q.y * q.y) + (q.z * q.z);
}      

[[nodiscard]] inline double norm(quat q) {
    return std::sqrt(norm_squared(q));
}

[[nodiscard]] inline quat normalized(quat q){
    assert(norm_squared(q) > 0.0 && "zero axis");

    double n  = norm(q);
    return quat(q.w/n, q.x / n, q.y/n, q.z/n);
}

// q = (cos θ/2, n·sin θ/2) where n is unit axis
[[nodiscard]] inline quat from_axis_angle(vec3 axis, double radians) {
    assert(norm_squared(axis) > 0.0 && "zero axis");

    double len_sq = norm_squared(axis);
    if (len_sq > 0.0 && !approx_equal(len_sq, 1.0)) {
        double inv_len = 1.0 / std::sqrt(len_sq);
        axis.x *= inv_len;
        axis.y *= inv_len;
        axis.z *= inv_len;
    }

    double half_angle = radians/2.0;
    double s = std::sin(half_angle);
    return quat(std::cos(half_angle), axis.x*s, axis.y * s, axis.z*s);
}

// v' = q v q* (embed v as pure quaternion (0,v), extract xyz)
[[nodiscard]] inline vec3 rotate(quat q, vec3 v){
    quat pure_v = quat(0.0, v.x, v.y, v.z);

    quat rot_q = q * pure_v * conjugate(q);

    return vec3(rot_q.x, rot_q.y, rot_q.z);
}

// Spherical linear interpolation, t ∈ [0, 1]
[[nodiscard]] inline quat slerp(quat a, quat b, double t){
    double dot = dot_product(a,b);
    if (dot < 0){
        b = -b;
        dot = -dot;
    }

    dot = std::clamp(dot, -1.0, 1.0);
    double theta = std::acos(dot);
    if (std::abs(theta) < 1e-6) {
        return normalized(a * (1.0 - t) + b * t);
    }

    double sin_theta = std::sin(theta);
    double weight_a = std::sin((1-t) * theta) / sin_theta;
    double weight_b = std::sin((t) * theta) / sin_theta;

    return a * weight_a + b * weight_b;
}

// Quaternion → mat3
[[nodiscard]] inline mat3 quat_to_mat3(quat q) {
    return {{
        1 - 2 * (q.y * q.y + q.z * q.z), 2* (q.x * q.y-q.w * q.z), 2 * (q.x * q.z + q.w * q.y),
        2 * (q.x * q.y + q.w * q.z), 1-2 * (q.x * q.x + q.z * q.z), 2 * (q.y * q.z-q.w * q.x),
        2 * (q.x * q.z - q.w * q.y), 2 * (q.y * q.z + q.w * q.x), 1 - 2 * (q.x * q.x + q.y * q.y)
    }};
}

// Mat3 → quaternion (Shepperd's method — has branches)
[[nodiscard]] inline quat mat3_to_quat(const mat3& R) {
    // 1. Evaluate trace variations to avoid division by zero
    double p0 = 1.0 + R(0,0) + R(1,1) + R(2,2);
    double p1 = 1.0 + R(0,0) - R(1,1) - R(2,2);
    double p2 = 1.0 - R(0,0) + R(1,1) - R(2,2);
    double p3 = 1.0 - R(0,0) - R(1,1) + R(2,2);

    // 2. Find the robust dominant component
    double max_p = std::max({p0, p1, p2, p3});

    quat q;
    if (max_p == p0) {
        q.w = 0.5 * std::sqrt(p0);
        double s = 0.25 / q.w;
        q.x = (R(2,1) - R(1,2)) * s;
        q.y = (R(0,2) - R(2,0)) * s;
        q.z = (R(1,0) - R(0,1)) * s;
    } else if (max_p == p1) {
        q.x = 0.5 * std::sqrt(p1);
        double s = 0.25 / q.x;
        q.w = (R(2,1) - R(1,2)) * s;
        q.y = (R(0,1) + R(1,0)) * s;
        q.z = (R(0,2) + R(2,0)) * s;
    } else if (max_p == p2) {
        q.y = 0.5 * std::sqrt(p2);
        double s = 0.25 / q.y;
        q.w = (R(0,2) - R(2,0)) * s;
        q.x = (R(0,1) + R(1,0)) * s;
        q.z = (R(1,2) + R(2,1)) * s;
    } else {
        q.z = 0.5 * std::sqrt(p3);
        double s = 0.25 / q.z;
        q.w = (R(1,0) - R(0,1)) * s;
        q.x = (R(0,2) + R(2,0)) * s;
        q.y = (R(1,2) + R(2,1)) * s;
    }
    return q;
}