#pragma once
#include <vector>
#include "math/sparse.hpp"

namespace simcore::cuda {
    [[nodiscard]] std::vector<double> solve_cg_gpu(
        const SparseMatrix& A,
        const std::vector<double>& b,
        std::vector<double> x,
        double tol = 1e-10,
        int max_iter = 1000
    );
}