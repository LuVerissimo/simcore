#pragma once
#include <vector>

using ReductionKernelPtr = void(*)(const double*, const double*, double*, unsigned int);

__global__ void naiveReductionKernel(const double* a,  const double* b, double* output, unsigned int N);
__global__ void sequentialAddressingReductionKernel(const double* a,  const double* b, double* output, unsigned int N);
__global__ void warpShuffleReductionKernel(const double* a,  const double* b, double* output, unsigned int N);

namespace simcore::cuda {
    [[nodiscard]] double launch_reduction(const std::vector<double>& a, const std::vector<double>& b, ReductionKernelPtr kernel);
    [[nodiscard]] double launch_reduction(const double* d_a, const double* d_b, int n, ReductionKernelPtr kernel);
}