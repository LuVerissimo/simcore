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

void bench_saxpy() {
    using simcore::cuda::CudaBuffer;
    for (int n : {1'000'000, 10'000'000, 100'000'000}) {\
        double cpu_total = 0, gpu_total = 0, kernel_total = 0;
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
            auto t0 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < n; i++) h_y_cpu[i] += a * h_x[i];
            auto t1 = std::chrono::high_resolution_clock::now();
            cpu_total += ms(t0, t1);

            //GPU (total: upload + kernel + download)
            t0 = std::chrono::high_resolution_clock::now();
            cudaMemcpy(d_x.data(), h_x.data(), size, cudaMemcpyHostToDevice);
            cudaMemcpy(d_y.data(), h_y_gpu.data(), size, cudaMemcpyHostToDevice);
            simcore::cuda::saxpy(d_x, d_y, a, n);
            cudaMemcpy(h_y_gpu.data(), d_y.data(), size, cudaMemcpyDeviceToHost);
            t1 = std::chrono::high_resolution_clock::now();
            gpu_total = ms(t0, t1);


            // Kernel only (data already on device from above, re-upload fresh)
            cudaMemcpy(d_y.data(), h_y_gpu.data(), size, cudaMemcpyHostToDevice);
            t0 = std::chrono::high_resolution_clock::now();
            simcore::cuda::saxpy(d_x, d_y, a, n);
            t1 = std::chrono::high_resolution_clock::now();
            kernel_total += ms(t0, t1);
        }
        double cpu_avg = cpu_total / runs;
        double gpu_avg = gpu_total / runs;
        double kernel_avg = kernel_total / runs;
        double transfer_avg = gpu_avg - kernel_avg;

        std::cout << "N=" << n
                  << " CPU=" << cpu_avg << "ms"
                  << " GPU(total)=" << gpu_avg << "ms"
                  << " Kernel=" << kernel_avg << "ms"
                  << " Transfer=" << transfer_avg << "ms"
                  << " Speedup(total)=" << cpu_avg / gpu_avg << "x"
                  << " Speedup(kernel)=" << cpu_avg / kernel_avg << "x\n";
    }
}

int main() {
    bench_saxpy();
}