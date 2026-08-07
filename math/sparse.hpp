#pragma once
#include "math/vec3.hpp"
#include <cmath>
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

// y = a + scalar * b
[[nodiscard]] inline std::vector<double> axpy(
    double scalar, const std::vector<double>& x,
    const std::vector<double>& y
) {
    std::vector<double> r(x.size());
    for (size_t i = 0; i < x.size(); ++i) r[i] = y[i] + scalar * x[i];
    return r;
}

[[nodiscard]] inline double dot(
    const std::vector<double>& a, 
    const std::vector<double>& b
) {
    double s = 0;
    for (size_t i = 0; i < a.size(); ++i) s += a[i] * b[i];
    return s;
}

[[nodiscard]] inline std::vector<double> solve_cg(
    const SparseMatrix& A,
    const std::vector<double>& b,
    std::vector<double> x,
    double tol = 1e-10,
    int max_iter = 1000
) {
    auto r = axpy(-1.0, spmv(A, x), b);
    auto p = r;
    for (auto k = 0; k < max_iter; ++k) {
      auto Ap = spmv(A,p);
      double rr = dot(r, r);
      auto alpha = rr / dot(p, Ap);

      x = axpy(alpha, p, x);
      auto r_new = axpy(-alpha, Ap, r);

      if (sqrt(dot(r_new,r_new)) < tol) break;

      auto beta = dot(r_new,r_new) / rr;
      p = axpy(beta, p,r_new);
      r = r_new;
    }
    return x;
}