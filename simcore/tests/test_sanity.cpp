// Sanity check: proves the build + test + CI pipeline works end to end.
// Week 1: delete this and replace with tests/test_vec3.cpp and tests/test_mat3.cpp
// that verify MATHEMATICS (R·Rᵀ = I, det(R) = 1, a·(a×b) = 0, ...), not plumbing.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("the harness is alive") {
    CHECK(2 + 2 == 4);
}

// Week 1 acceptance criterion, ready and waiting. Uncomment when vec3 exists:
//
// #include "math/vec3.hpp"
// static_assert(vec3{1,0,0}.dot(vec3{0,1,0}) == 0.0,
//               "vec3 math must run at compile time");
