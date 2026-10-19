#include <cuda_runtime.h>
#include <vector>
#include <stdexcept>
#include <string>
#include <functional>
#include "simcore/cuda/device_buffer.hpp"
#include "simcore/cuda/reduction.cuh"


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
__global__ void sequentialAddressingReductionKernel(const double* a,  const double* b, double* output, unsigned int N){
    extern __shared__ double sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

    // load element-wise product into shared mem
    sdata[tid] = (i < N) ? a[i] * b[i] : 0.0;
    __syncthreads();

    for (unsigned int stride = blockDim.x/2; stride > 0;  stride /= 2) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }
    if (tid == 0) {
        atomicAdd(output, sdata[0]);
    }
}
__global__ void warpShuffleReductionKernel(const double* a,  const double* b, double* output, unsigned int N){
    extern __shared__ double sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

    // Initial Element
    double val = (i < N) ? a[i] * b[i] : 0.0;

    // Reduce within the local warp
    unsigned int lane = tid % 32;
    unsigned int warpId = tid / 32;

    for (int offset = 16; offset > 0; offset /= 2) {
        val += __shfl_down_sync(0xffffffff, val, offset);
    }

    // write warp results to shared mem
    if (lane == 0) sdata[warpId] = val;
    __syncthreads();

    // final recution using first warp
    if (warpId == 0) {
        unsigned int totalWarps = blockDim.x / 32;
        val = (lane < totalWarps) ? sdata[lane] : 0.0;

        for (int offset = 16; offset > 0; offset /=2) {
            val += __shfl_down_sync(0xffffffff, val, offset);
        }
        if (tid == 0) {
            atomicAdd(output, val);
        }
    }
}

using simcore::cuda::CudaBuffer;
namespace simcore::cuda {

    [[nodiscard]] double launch_reduction(
        const std::vector<double>& a, 
        const std::vector<double>& b, 
        ReductionKernelPtr targetReductionKernel
    ) {
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

        targetReductionKernel<<<blocksPerGrid, threadsPerBlock, sharedMemSize>>>(d_a.data(), d_b.data(), d_out.data(), N);

        cudaError_t errSync = cudaDeviceSynchronize();
        if (errSync != cudaSuccess) {
            throw std::runtime_error("Dot Product Reduction kernel failed: " + std::string(cudaGetErrorString(errSync)));
        }

        // copy to host
        cudaMemcpy(&h_out, d_out.data(), sizeof(double), cudaMemcpyDeviceToHost);
        return h_out;
    }

    [[nodiscard]] double launch_reduction(
        const double* d_a, 
        const double* d_b, 
        int n,
        ReductionKernelPtr targetReductionKernel
    ) {
        // device setup
        CudaBuffer<double> d_out(1);
        double h_out = 0.0;
        cudaMemcpy(d_out.data(), &h_out, sizeof(double), cudaMemcpyHostToDevice);

        size_t threadsPerBlock = 256;
        size_t blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;
        size_t warpsPerBlock = (threadsPerBlock + 31) / 32; 
        size_t sharedMemSize =  warpsPerBlock * sizeof(double);
        
        targetReductionKernel<<<blocksPerGrid, threadsPerBlock, sharedMemSize>>>(d_a, d_b, d_out.data(), n);
       
        cudaError_t errSync = cudaDeviceSynchronize();
        if (errSync != cudaSuccess) {
            throw std::runtime_error("Reduction kernel failed: " + std::string(cudaGetErrorString(errSync)));
        }
    
        cudaMemcpy(&h_out, d_out.data(), sizeof(double), cudaMemcpyDeviceToHost);
        return h_out;
    }
}