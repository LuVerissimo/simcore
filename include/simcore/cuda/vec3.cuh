#pragma once
#include "vec3.hpp"
#include "simcore/cuda/device_buffer.hpp"


namespace simcore::cuda {
    void vec3_add(CudaBuffer<vec3f>& A, CudaBuffer<vec3f>& B, CudaBuffer<vec3f>& C, int n);
}
