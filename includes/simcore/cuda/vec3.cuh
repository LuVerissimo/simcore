#pragma once
#include "vec3.hpp"

__global__
void vecAddKernel(const vec3f* A, const vec3f* B, vec3f* C, int n) {
    int i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i < n) {
        C[i] = A[i] + B[i];
    }
}

