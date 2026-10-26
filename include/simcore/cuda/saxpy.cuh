#pragma once
#include "simcore/cuda/device_buffer.hpp"

namespace simcore::cuda {
    void saxpy(CudaBuffer<float>& x, CudaBuffer<float>& y, float a, int n);
    void daxpy(CudaBuffer<double>& x, CudaBuffer<double>& y, CudaBuffer<double>& z, double a, int n);
}