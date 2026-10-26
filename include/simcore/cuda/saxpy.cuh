#pragma once
#include "simcore/cuda/device_buffer.hpp"

namespace simcore::cuda {
    void saxpy(CudaBuffer<float>& x, CudaBuffer<float>& y, float a, int n);
    void daxpy(const double* x, const double* y, double* z, double a, int n);
}