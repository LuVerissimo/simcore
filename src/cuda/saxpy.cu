#include "simcore/cuda/saxpy.cuh"

__global__ void saxpyKernel(float* x, float* y, float a, int n) {
    int i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i < n) {
        y[i] += a * x[i];
    }
}

__global__ void daxpyKernel(const double* x, const double* y, double* z, double a, int n) {
    int i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i < n) {
        z[i] = a * x[i] + y[i];
    }
}

namespace simcore::cuda {
    void saxpy(CudaBuffer<float>& x, CudaBuffer<float>& y, float a, int n) {
        int threadsPerBlock = 256;
        int blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;
        saxpyKernel<<<blocksPerGrid, threadsPerBlock>>>(x.data(), y.data(), a, n);

        cudaError_t errSync = cudaDeviceSynchronize();
        if (errSync != cudaSuccess) {
            throw std::runtime_error("SAXPY kernel failed: " + std::string(cudaGetErrorString(errSync)));
        }
    }

    void daxpy(const double* x, const double* y, const double* z, double a, int n) {
        int threadsPerBlock = 256;
        int blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;
        daxpyKernel<<<blocksPerGrid, threadsPerBlock>>>(x, y, z, a, n);

        cudaError_t errSync = cudaDeviceSynchronize();
        if (errSync != cudaSuccess) {
            throw std::runtime_error("DAXPY kernel failed: " + std::string(cudaGetErrorString(errSync)));
        }
    }
}