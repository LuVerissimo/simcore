#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "math/sparse.hpp"
#include "math/vec3.hpp"
#include "doctest/doctest.h"
#include <cmath>
#include "simcore/cuda/spmv.cuh"

TEST_CASE("identity mat3 in CSR") {
    SparseMatrix I(3,3);
    I.values = {1, 1, 1};
    I.col_idx = {0, 1, 2};
    I.row_ptr = {0, 1, 2, 3};
    std::vector<double> y = spmv(I, {2,3,4});
    std::vector<double> z = {2,3,4};
    CHECK(approx_equal(y, z));
}

TEST_CASE("2x2 SPD system that I solved by hand") {
    SparseMatrix A(2, 2);
    A.values = {2, 1, 1, 3};
    A.col_idx = {0, 1, 0, 1};
    A.row_ptr = {0, 2, 4};

    std::vector<double> b = {1, 2};
    std::vector<double> x0 = {0, 0};
    auto x = solve_cg(A, b, x0);

    CHECK(approx_equal(x, {0.2, 0.6}));
}

TEST_CASE("GPU CG matches CPU CG on 2D Laplace to 1e-10")) {
    int N = 100;
    CSRMatrix csr = generateLaplace2DCSR(N);
    int numRows = N * N;

    SparseMatrix A(numRows, numRows);
    A.values = csr.values;
    A.col_idx = csr.colIdx;
    A.row_ptr = csr.rowPtrs;

    std::vector<double> b(numRows, 1.0);
    std::vector<double> x0(numRows, 0.0);

    auto x_cpu = solve_cg(A, b, x0);
    auto x_gpu = solve_cg_gpu(A, b, x0);

    CHECK(approx_equal(x_cpu, x_gpu));
}