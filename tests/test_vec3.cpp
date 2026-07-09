#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "math/vec3.hpp"
#include "math/mat3.hpp" 
#include "doctest/doctest.h"

#include <cmath>
#include <numbers>
#include <vector>
#include <utility>
#include <random>


TEST_CASE("cross prod for problem 2") {
    constexpr vec3 a{2, -1, 3}, b{1,4,-2};
    CHECK(approx_equal(cross(a,b), vec3{-10, 7, 9}));
    CHECK(approx_equal(dot(cross(a,b), a), 0.0)); //orthogonality, prob 2
    CHECK(approx_equal(dot(cross(a,b), b), 0.0));
}

TEST_CASE("projection for problem 3") {
    constexpr vec3 a{2, -1, 3}, b{1, 4,-2};
    const vec3 proj = project_onto(a,b);
    CHECK(approx_equal(proj, vec3{-8.0/21, -32.0/21, 16.0/21}));
    CHECK(approx_equal(dot(proj, a - proj), 0.0));   // proj ⊥ remainder
}

TEST_CASE("rotation acts correctly for problem 7") {
    const vec3 p{1, 2, 3};
    CHECK(approx_equal(rotation_z(PI/2) * p, vec3{-2, 1, 3}));
}

TEST_CASE("rotation properties") {
    const mat3 R = rotation_z(std::numbers::pi / 3);
    CHECK(approx_equal(R * transpose(R), mat3::identity()));
    CHECK(approx_equal(determinant(R), 1.0));
}

// Problems with algebraic structure become PROPERTY tests over random inputs:

std::vector<std::pair<vec3, vec3>> random_vec3_pairs(int n) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> d(-10.0, 10.0);
    std::vector<std::pair<vec3, vec3>> out;
    out.reserve(n);
    for (int i = 0; i < n; ++i)
        out.push_back({{d(gen), d(gen), d(gen)}, {d(gen), d(gen), d(gen)}});
    return out;
}

TEST_CASE("a·(a×b) = 0 for random vectors") {
    for (auto [a, b] : random_vec3_pairs(100))
        CHECK(std::abs(dot(a, cross(a, b))) < 1e-12);
}