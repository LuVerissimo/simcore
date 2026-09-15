#pragma once
#include "vec3.hpp"
#include <stdexcept>
#include <string>

__global__
void vecAddKernel(const vec3f* A, const vec3f* B, vec3f* C, int n) {
    int i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i < n) {
        C[i] = A[i] + B[i];
    }
}

//host by default so no need to set __host__, almost omitting use of '_h_' in variable
void vec3Add(const vec3f* A, const vec3f* B, vec3f* C, int n) {
    // Part 1: Allocate device memory for A, B, and C
    int size = n * sizeof(vec3f);
    vec3f *A_d = nullptr, *B_d = nullptr, *C_d = nullptr;

    // Copy A and B to device memory
    cudaError_t errA = cudaMalloc((void **)&A_d, size);
    cudaError_t errB = cudaMalloc((void **)&B_d, size);
    cudaError_t errC = cudaMalloc((void **)&C_d, size);

    if (errA != cudaSuccess || errB != cudaSuccess || errC != cudaSuccess)  {
        if (A_d) cudaFree(A_d);
        if (B_d) cudaFree(B_d);
        if (C_d) cudaFree(C_d);
        throw std::runtime_error("CUDA Malloc failed.");
    }
    cudaMemcpy(A_d, A, size, cudaMemcpyHostToDevice);
    cudaMemcpy(B_d, B, size, cudaMemcpyHostToDevice);

    // Part 2: Call kernel/launch a grid of threads to perform the actual vector addition
    int threadsPerBlock = 256;
    int blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;

    vecAddKernel<<<blocksPerGrid, threadsPerBlock>>>(A_d, B_d, C_d, n);
    
    // Part 3: Copy C from the device memory & Free device vectors
    cudaError_t errSync = cudaGetLastError();
    
    if (errSync != cudaSuccess) {
        cudaFree(A_d); cudaFree(B_d); cudaFree(C_d);
        throw std::runtime_error("Kernel Launch failed: " + std::string(cudaGetErrorString(errSync)));
    }
    cudaMemcpy(C , C_d, size, cudaMemcpyDeviceToHost);

    cudaFree(A_d);
    cudaFree(B_d);
    cudaFree(C_d);
}
