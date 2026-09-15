#pragma once
#include "simcore/cuda/device_buffer.hpp"

namespace simcore::cuda {
    void saxpy(CudaBuffer<float>& x, CudaBuffer<float>& y, float a, int n);
}