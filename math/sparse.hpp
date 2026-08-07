#pragma once
#include "math/vec3.hpp"
#include <vector>
#include <cassert>


struct SparseMatrix {
    int rows, cols;
    std::vector<double> values; //nnz values
    std::vector<int> col_idx;   // col idx each entry
    std::vector<int> row_ptr;   // row_ptr[i] = start of row i in values

    SparseMatrix(int r, int c) : rows(r), cols(c), row_ptr(r + 1, 0) {}

    // Collets entries as (row, col, value) triple, then build CSR
    static SparseMatrix from_triplets(int rows, int cols,
        std::vector<int>& tri_row,
        std::vector<int>& tri_col,
        std::vector<double>& tri_val
    );

};

[[nodiscard]] inline std::vector<double> spmv(const SparseMatrix& A, const std::vector<double>& x) {
    std::vector<double> y(A.rows, 0.0);

    for (auto i = 0; i < A.rows; ++i) {
        y[i] = 0;
        for (auto k = A.row_ptr[i]; k < A.row_ptr[i + 1]; ++k) {
            y[i] += A.values[k] * x[A.col_idx[k]];
        }
    }
    return y;
}

[[nodiscard]] inline bool approx_equal(
    const std::vector<double>& a, 
    const std::vector<double>& b,
    double eps = 1e-9 
) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (cabs(a[i] - b[i]) > eps) return false;
    }
    return true;
}

