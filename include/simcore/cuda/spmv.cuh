#pragma once
#include <vector>

struct CSRMatrix {
    int numRows;
    int numCols;
    std::vector<double> values;
    std::vector<int> colIdx;
    std::vector<int> rowPtrs;
};

inline CSRMatrix generateLaplace2DCSR(int gridDim) {
    CSRMatrix csr;
    int N = gridDim;
    int numRows = N * N; // 1,000,000 for a 1000x1000 grid
    
    csr.numRows = numRows;
    csr.numCols = numRows;
    
    // Pre-allocate row pointers
    csr.rowPtrs.resize(numRows + 1, 0);
    
    // Step 1: Count non-zeros per row to pre-allocate memory (Optimizes allocation overhead)
    size_t totalNNZ = 0;
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            int nnzInRow = 1; // Center element (diagonal) Always present
            if (c > 0)     nnzInRow++; // Left neighbor
            if (c < N - 1) nnzInRow++; // Right neighbor
            if (r > 0)     nnzInRow++; // Up neighbor
            if (r < N - 1) nnzInRow++; // Down neighbor
            totalNNZ += nnzInRow;
        }
    }
    
    csr.values.reserve(totalNNZ);
    csr.colIdx.reserve(totalNNZ);

    // Step 2: Populate the CSR structural arrays
    int currentNNZ = 0;
    csr.rowPtrs[0] = 0;

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            int rowIdx = r * N + c;

            // 1. Up neighbor (row - 1)
            if (r > 0) {
                csr.values.push_back(-1.0);
                csr.colIdx.push_back((r - 1) * N + c);
            }
            // 2. Left neighbor (col - 1)
            if (c > 0) {
                csr.values.push_back(-1.0);
                csr.colIdx.push_back(r * N + (c - 1));
            }
            // 3. Center diagonal element
            csr.values.push_back(4.0);
            csr.colIdx.push_back(rowIdx);
            // 4. Right neighbor (col + 1)
            if (c < N - 1) {
                csr.values.push_back(-1.0);
                csr.colIdx.push_back(r * N + (c + 1));
            }
            // 5. Down neighbor (row + 1)
            if (r < N - 1) {
                csr.values.push_back(-1.0);
                csr.colIdx.push_back((r + 1) * N + c);
            }

            // Track row bounds
            csr.rowPtrs[rowIdx + 1] = csr.values.size();
        }
    }

    return csr;
}


namespace simcore::cuda {
    void spmvCSR(const int* rowPtrs, const int* colIdx, const double* values, const double* x, double* y, int numRows);
}