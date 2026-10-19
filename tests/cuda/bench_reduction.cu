#include "simcore/cuda/reduction.cuh"
#include "simcore/cuda/device_buffer.hpp"
#include <chrono>
#include <cuda_runtime.h>
#include <vector>
#include <cassert>
#include <iostream>

auto ms = [](auto a, auto b) {
    return std::chrono::duration<double, std::milli>(b-a).count();
};

void bench_reduction() {
    using simcore::cuda::CudaBuffer;
    for (int n : {10'000'000, 100'000'000}) {
        double cpu_total = 0, gpu_total = 0, kernel_total = 0;
        int runs = 5;
        int size = n * sizeof(double);
        for (int r = 0; r < runs; r++) {
            
            //Setup Host
            std::vector<double> h_a(n, 1.0); 
            std::vector<double> h_b(n, 2.0); 

            //Setup Device 
            CudaBuffer<double> d_a(n);
            CudaBuffer<double> d_b(n);

            //CPU
            auto t0 = std::chrono::high_resolution_clock::now();
            double cpu_dot = 0.0;
            for (int i = 0; i < n; i++) {
                cpu_dot += h_b[i] * h_a[i];
            }
            auto t1 = std::chrono::high_resolution_clock::now();
            cpu_total += ms(t0, t1);

            //GPU (total: upload + kernel + download)
            t0 = std::chrono::high_resolution_clock::now();
            cudaMemcpy(d_a.data(), h_a.data(), size, cudaMemcpyHostToDevice);
            cudaMemcpy(d_b.data(), h_b.data(), size, cudaMemcpyHostToDevice);

            double gpu_dot = simcore::cuda::naive_reduction_dot(d_a.data(), d_b.data(), n);
            t1 = std::chrono::high_resolution_clock::now();
            gpu_total += ms(t0, t1);

            std::cout << " CPU_dot=" << cpu_dot << " GPU_dot=" << gpu_dot << "\n";

            // Kernel only (data already on device from above, re-upload fresh)
            t0 = std::chrono::high_resolution_clock::now();
            simcore::cuda::naive_reduction_dot(d_a.data(), d_b.data(), n);
            t1 = std::chrono::high_resolution_clock::now();
            kernel_total += ms(t0, t1);
        }
    
        double cpu_avg = cpu_total / runs;
        double gpu_avg = gpu_total / runs;
        double kernel_avg_seconds = (kernel_total / runs) / 1000;
        double bandwidth = 2 * n * sizeof(double) / (kernel_avg_seconds) / 1e9 ;
        
        std::cout << "N=" << n
                    << " CPU=" << cpu_avg << "ms"
                    << " GPU(total)=" << gpu_avg << "ms"
                    << " Kernel=" << kernel_avg_seconds << "s"
                    << " Bandwidth=" << bandwidth << "GB/s\n";
    }
}

int main() {
    bench_reduction();
}