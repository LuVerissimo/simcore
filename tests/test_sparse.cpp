#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "math/sparse.hpp"
#include "math/vec3.hpp"
#include "doctest/doctest.h"
#include <cmath>


TEST_CASE("identity mat3 in CSR") {
    SparseMatrix I(3,3);
    I.values = {1, 1, 1};
    I.col_idx = {0, 1, 2};
    I.row_ptr = {0, 1, 2, 3};
    std::vector<double> y = spmv(I, {2,3,4});
    std::vector<double> z = {2,3,4};
    CHECK(approx_equal(y, z));
}