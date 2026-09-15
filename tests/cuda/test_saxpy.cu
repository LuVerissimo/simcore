#include "simcore/cuda/saxpy.cuh"
#include "simcore/cuda/device_buffer.hpp"
#include <vector>
#include <cassert>
#include <iostream>

void test_saxpy(int n, float a) {
    using simcore::cuda::CudaBuffer;
    std::vector<float> h_x(n, 1.0f);
    std::vector<float> h_y(n, 2.0f);

    CudaBuffer<float> d_x(n);   
    CudaBuffer<float> d_y(n);
    int size = n * sizeof(float);

    cudaMemcpy(d_x.data(), h_x.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_y.data(), h_y.data(), size, cudaMemcpyHostToDevice);
    
    simcore::cuda::saxpy(d_x, d_y, a, n);
    
    cudaMemcpy(h_y.data(), d_y.data(), size, cudaMemcpyDeviceToHost);

    for (int i = 0; i < n; i++) {
        assert(h_y[i] == a * 1.0f + 2.0f);
    }

    std::cout << "saxpy test passed.\n";
}

int main() {
    test_saxpy(1024, 2.0f);
    return 0;
}