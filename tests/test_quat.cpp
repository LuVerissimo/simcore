#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "math/mat3.hpp"
#include "math/quat.hpp"
#include "math/vec3.hpp"
#include "doctest/doctest.h"
#include <cmath>
#include <iostream>

TEST_CASE("rotation of axis angle ≈ {0,1,0}") {
    quat aa = from_axis_angle({0,0,1}, PI/2);
    CHECK(approx_equal(rotate(aa, {1,0,0}), {0,1,0}));
}

TEST_CASE("quat_to_mat3(from_axis_angle({0,0,1}, PI/3)) ≈ rotation_z(π/3)"){
    quat faa = from_axis_angle({0,0,1}, PI/3);
    mat3 qtm3 = quat_to_mat3(faa);
    mat3 R = rotation_z(PI/3);
    CHECK(approx_equal(qtm3, R));
}

TEST_CASE("Round-trip: mat3_to_quat(quat_to_mat3(q)) ≈ q (or −q — both represent the same rotation)"){
    quat q = {1.0, 1.0, 1.0, 1.0};
    
    mat3 qtm3 = quat_to_mat3(q);

    quat rt = mat3_to_quat(qtm3);

    bool match = approx_equal(q, rt) || approx_equal(q, -rt);

    std::cout << "q:  " << q.w << " " << q.x << " " << q.y << " " << q.z << "\n";
    std::cout << "rt: " << rt.w << " " << rt.x << " " << rt.y << " " << rt.z << "\n";
    std::cout << "-rt: " << -rt.w << " " << -rt.x << " " << -rt.y << " " << -rt.z << "\n";
    CHECK(match);
}

TEST_CASE("Slerp endpoints: slerp(a, b, 0) ≈ a, slerp(a, b, 1) ≈ b") {
    quat qa = from_axis_angle({0,0,1}, 0.3);
    quat qb = from_axis_angle({0,1,0}, 1.2);

    quat slerp_a = slerp(qa, qb, 0);
    quat slerp_b = slerp(qa, qb, 1);

    std::cout << "qa: " << qa.w << " " << qa.x << " " << qa.y << " " << qa.z << "\n";
    std::cout << "s0: " << slerp_a.w << " " << slerp_a.x << " " << slerp_a.y << " " << slerp_a.z << "\n";
    
    CHECK(approx_equal(qa, slerp_a));
    CHECK(approx_equal(qb, slerp_b));
}