
#include "simcore/cuda/spmv.cuh"
#include <stdexcept>

__global__
void spmvCSRKernel(const int* rowPtrs, const int* colIdx, const double* values, double* x, double* y, int numRows) {
    unsigned int row = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < numRows) {
        double sum = 0.0f;
        for (unsigned int i = rowPtrs[row]; i < rowPtrs[row + 1]; ++i) {
            unsigned int col = colIdx[i];
            double value = values[i];
            sum += x[col] * value;
        }
        y[row] = sum;
    }

}

namespace simcore::cuda {
    void spmvCSR(const int* rowPtrs, const int* colIdx, const double* values, double* x, double* y, int numRows) {
        int threadsPerBlock = 256;
        int blocksPerGrid = (numRows + threadsPerBlock - 1) / threadsPerBlock;
        spmvCSRKernel<<<blocksPerGrid, threadsPerBlock>>>(rowPtrs, colIdx, values, x, y, numRows);

        cudaError_t errSync = cudaDeviceSynchronize();
        if (errSync != cudaSuccess) {
            throw std::runtime_error("SpMV-CSR kernel failed: " + std::string(cudaGetErrorString(errSync)));
        }
    }
}