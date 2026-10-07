#pragma once

namespace simcore::cuda {
    void spmvCSR(const int* rowPtrs, const int* colIdx, const double* values, double* x, double* y, int numRows);
}