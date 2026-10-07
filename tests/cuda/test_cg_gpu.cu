#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <cassert>
#include <cmath>
#include <utility>
#include <iostream>
#include "simcore/cuda/cg_gpu.cuh"
#include "simcore/cuda/spmv.cuh"
#include "math/sparse.hpp"


TEST_CASE("GPU CG matches CPU CG on 2D Laplace to 1e-10") {
    using simcore::cuda::solve_cg_gpu;
    
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