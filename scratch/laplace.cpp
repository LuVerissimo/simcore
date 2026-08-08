

#include "math/sparse.hpp"
#include <iostream>
#include <vector>

// Build the 5-point Laplace stencil for N×N interior grid
// Grid point (i,j) maps to row i*N+j in the matrix
SparseMatrix build_laplace(int N) {
    int n = N * N;
    SparseMatrix A(n, n);

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int idx = i * N + j;
            A.row_ptr[idx] = A.values.size();

            if (i > 0)     { A.values.push_back(-1); A.col_idx.push_back(idx - N); }
            if (j > 0)     { A.values.push_back(-1); A.col_idx.push_back(idx - 1); }
            A.values.push_back(4);  A.col_idx.push_back(idx);
            if (j < N - 1) { A.values.push_back(-1); A.col_idx.push_back(idx + 1); }
            if (i < N - 1) { A.values.push_back(-1); A.col_idx.push_back(idx + N); }
        }
    }
    A.row_ptr[n] = A.values.size();
    return A;
}

int main() {
    int N = 50;
    auto A = build_laplace(N);
    std::vector<double> b(N * N, 0.0);

    // Top row boundary: add 100 to RHS for grid points adjacent to top
    for (int j = 0; j < N; ++j) b[j] += 100.0;

    auto x = solve_cg(A, b, std::vector<double>(N * N, 0.0));

    // Print solution as grid
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            std::cout << x[i * N + j] << "\t";
        std::cout << "\n";
    }
}