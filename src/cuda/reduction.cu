#include <cuda_runtime.h>
#include <vector>
#include <stdexcept>
#include <string>
#include "simcore/cuda/device_buffer.hpp"

__global__ void naiveReductionKernel(const double* a,  const double* b, double* output, unsigned int N) {
    // dynamically allocated shared mem
    extern __shared__ double sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

    // load element-wise product into shared mem
    sdata[tid] = (i < N) ? a[i] * b[i] : 0.0;
    __syncthreads();

    for (unsigned int stride = 1; stride < blockDim.x;  stride *= 2) {
        if (tid % (2 * stride) == 0 ) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }
    if (tid == 0) {
        atomicAdd(output, sdata[0]);
    }
}
__global__ void sequentialAddressingReductionKernel(double* input, double* output){}
__global__ void warpShuffleReductionKernel(double* input, double* output){}


using simcore::cuda::CudaBuffer;

namespace simcore::cuda {
    [[nodiscard]] double naive_reduction_dot(const std::vector<double>& a, const std::vector<double>& b) {
        if (a.size() != b.size()) throw std::invalid_argument("vec sizes aren't equal");
        
        unsigned int N = a.size();
        size_t sizeBytes = N * sizeof(double);

        // device setup
        CudaBuffer<double> d_a(N);
        CudaBuffer<double> d_b(N);
        CudaBuffer<double> d_out(1);
        double h_out = 0.0;

        // copy to device
        cudaMemcpy(d_a.data(), a.data(), sizeBytes, cudaMemcpyHostToDevice);
        cudaMemcpy(d_b.data(), b.data(), sizeBytes, cudaMemcpyHostToDevice);
        cudaMemcpy(d_out.data(), &h_out, sizeof(double), cudaMemcpyHostToDevice);

        size_t threadsPerBlock = 256;
        size_t blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;
        size_t sharedMemSize = threadsPerBlock * sizeof(double);

        naiveReductionKernel<<<blocksPerGrid, threadsPerBlock, sharedMemSize>>>(d_a.data(), d_b.data(), d_out.data(), N);
       
        cudaError_t errSync = cudaDeviceSynchronize();
        if (errSync != cudaSuccess) {
            throw std::runtime_error("Dot Product Reduction kernel failed: " + std::string(cudaGetErrorString(errSync)));
        }

        // copy to host
        cudaMemcpy(&h_out, d_out.data(), sizeof(double), cudaMemcpyDeviceToHost);

        return h_out;
    }
    void sequential_addressing_reduction(double* input, double* output);
    void warp_shuffle_reduction(double* input, double* output);
}