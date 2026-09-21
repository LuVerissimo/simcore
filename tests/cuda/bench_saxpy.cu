#include "simcore/cuda/saxpy.cuh"
#include "simcore/cuda/device_buffer.hpp"
#include <chrono>
#include <cuda_runtime.h>
#include <vector>
#include <cassert>
#include <iostream>

auto ms = [](auto a, auto b) {
    return std::chrono::duration<double, std::milli>(b-a).count();
};

double acc_gpu_time;
double acc_cpu_time;

void bench_saxpy() {
    using simcore::cuda::CudaBuffer;
    for (int n : {1'000'000, 10'000'000, 100'000'000}) {\
        double cpu_total = 0, gpu_total = 0;
        int runs = 5;
        int size = n * sizeof(float);
        float a = 2.0f;

        for (int r = 0; r < runs; r++) {
            //Setup
            std::vector<float> h_x(n, 1.0f);
            std::vector<float> h_y_cpu(n, 2.0f);
            std::vector<float> h_y_gpu(n, 2.0f);
            CudaBuffer<float> d_x(n);   
            CudaBuffer<float> d_y(n);
    
            //CPU
            auto start_cpu = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < n; i++) h_y_cpu[i] += a * h_x[i];
            auto stop_cpu = std::chrono::high_resolution_clock::now();
            auto cpu_total = ms(start_cpu, stop_cpu);

            //GPU
            auto start_gpu = std::chrono::high_resolution_clock::now();
            cudaMemcpy(d_x.data(), h_x.data(), size, cudaMemcpyHostToDevice);
            cudaMemcpy(d_y.data(), h_y_gpu.data(), size, cudaMemcpyHostToDevice);
            simcore::cuda::saxpy(d_x, d_y, a, n);
            cudaMemcpy(h_y_gpu.data(), d_y.data(), size, cudaMemcpyDeviceToHost);
            auto stop_gpu = std::chrono::high_resolution_clock::now();
            auto gpu_time = ms(start_gpu, stop_gpu);
        }

        std::cout << "N=" << n
                  << " CPU=" << cpu_total/runs << "ms"
                  << " GPU=" << gpu_total/runs << "ms"
                  << " Speedup=" << cpu_total/gpu_total << "x\n";
    }

}

int main() {
    bench_saxpy();
}