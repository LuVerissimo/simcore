#pragma once

#include <array>
#include <cmath>
#include "vec3.hpp"


struct mat3 {
    std::array<double, 9> m{};   // {} → zero-initialized

    constexpr double  operator()(int r, int c) const { return m[3 * r + c]; }
    constexpr double& operator()(int r, int c)       { return m[3 * r + c]; }

    [[nodiscard]] constexpr vec3 col(int c) const { return {m[c], m[3 + c], m[6 + c]}; }
    [[nodiscard]] constexpr vec3 row(int r) const { return {m[3*r], m[3*r + 1], m[3*r + 2]}; }

    [[nodiscard]] static constexpr mat3 identity() {
        mat3 I;
        I(0,0) = I(1,1) = I(2,2) = 1.0;
        return I;
    }
};

// Mat × vec as LINEAR COMBINATION OF COLUMNS — the code encodes the math:
// Ax = x·col₀ + y·col₁ + z·col₂.
[[nodiscard]] constexpr vec3 operator*(const mat3& A, vec3 v) {
    return v.x * A.col(0) + v.y * A.col(1) + v.z * A.col(2);
}

// ─────────────────────────────────────────────────────────────────────────
//  build each factory by asking "where do x̂, ŷ, ẑ go?"
// ─────────────────────────────────────────────────────────────────────────

// col_j(AB) = A · col_j(B). Write it that way.
[[nodiscard]] constexpr mat3 operator*(const mat3& A, const mat3& B) {

    vec3 col0 = A * B.col(0);
    vec3 col1 = A * B.col(1);
    vec3 col2 = A * B.col(2);

    return {{
        col0.x, col1.x, col2.x,
        col0.y, col1.y, col2.y,
        col0.z, col1.z, col2.z
    }};
}

[[nodiscard]] constexpr mat3 transpose(const mat3& A) {
    return {{   
        A(0,0), A(1,0), A(2,0),
        A(0,1), A(1,1), A(2,1),
        A(0,2), A(1,2), A(2,2) 
    }};
}

// Via the scalar triple product of the columns: det = c₀ · (c₁ × c₂).
[[nodiscard]] constexpr double determinant(const mat3& A) {
    return dot(A.col(0), cross(A.col(1), A.col(2))); 
}

// Angle in radians, CCW positive, column convention.
[[nodiscard]] inline mat3 rotation_x(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return  {
        1.0, 0.0, 0.0, 
        0.0, c, -s,
        0.0, s, c
    };
}
[[nodiscard]] inline mat3 rotation_y(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return  {
        c,  0.0,  s,
        0.0, 1.0, 0.0,
        -s,  0.0,  c   
    };
}
[[nodiscard]] inline mat3 rotation_z(double theta) {
    double c = std::cos(theta);
    double s = std::sin(theta);
    return  {
        c, -s, 0.0,
        s, c, 0.0,
        0.0, 0.0, 1.0
    };
}

[[nodiscard]] constexpr bool approx_equal(const mat3& A, const mat3& B,
                                          double eps_abs = 1e-9,
                                          double eps_rel = 1e-9) {
    for (int i = 0; i < 9; ++i)
        if (!approx_equal(A.m[i], B.m[i], eps_abs, eps_rel)) return false;
    return true;
}
