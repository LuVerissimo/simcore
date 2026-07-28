#pragma once
#include "vec3.hpp"
#include <cmath>
#include <cassert>

struct quat {
    double w{1.0}, x{0.0}, y{0.0}, z{0.0};  // default = identity rotation
};

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
[[nodiscard]] inline quat slerp(quat a, quat b, double t);

// Quaternion → mat3 (formula on week-02 sheet)
[[nodiscard]] inline mat3 quat_to_mat3(quat q);

// Mat3 → quaternion (Shepperd's method — has branches)
[[nodiscard]] inline quat mat3_to_quat(const mat3& m);