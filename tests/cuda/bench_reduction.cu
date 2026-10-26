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

void bench_reduction(ReductionKernelPtr kernel, size_t threadsPerBlock) {
    using simcore::cuda::CudaBuffer;
    for (int n : {10'000'000, 100'000'000}) {
        double cpu_total = 0, gpu_total = 0, kernel_total_ms = 0;
        int runs = 5;
        size_t size = n * sizeof(double);

        // CUDA Event handles for microsecond-accurate kernel timing
        cudaEvent_t start, stop;
        cudaEventCreate(&start);
        cudaEventCreate(&stop);

        for (int r = 0; r < runs; r++) {
            
            //Setup Host
            std::vector<double> h_a(n, 1.0); 
            std::vector<double> h_b(n, 2.0); 

            //Setup Device 
            CudaBuffer<double> d_a(n);
            CudaBuffer<double> d_b(n);
            CudaBuffer<double> d_out(1);

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

            double gpu_dot = simcore::cuda::launch_reduction(d_a.data(), d_b.data(), n, threadsPerBlock, kernel);
            t1 = std::chrono::high_resolution_clock::now();
            gpu_total += ms(t0, t1);

            if (r == 0) {
                std::cout << "  [Verification] CPU_dot=" << cpu_dot << " GPU_dot=" << gpu_dot << "\n";
            }

            //Kernel Only Bench
            double zero = 0.0;
            cudaMemcpy(d_out.data(), &zero, sizeof(double), cudaMemcpyHostToDevice);

            size_t blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;

            // shared mem selection mirroring .cu dynamica imple
            size_t sharedMemSize = 0;
            if (kernel == warpShuffleReductionKernel) {
                size_t warpsPerBlock = (threadsPerBlock + 31) / 32;
                sharedMemSize = warpsPerBlock * sizeof(double);
            } else {
                sharedMemSize = threadsPerBlock * sizeof(double);
            }

             // Record start event on the GPU stream
            cudaEventRecord(start);
            
            kernel<<<blocksPerGrid, threadsPerBlock, sharedMemSize>>>(d_a.data(), d_b.data(), d_out.data(), n);
            
            // Record stop event right after execution tracking
            cudaEventRecord(stop);
            cudaEventSynchronize(stop); // Wait specifically for this kernel execution thread to clear

            float milliseconds = 0;
            cudaEventElapsedTime(&milliseconds, start, stop);
            kernel_total_ms += milliseconds;
        }
    
        double cpu_avg = cpu_total / runs;
        double gpu_avg = gpu_total / runs;
        
        double kernel_avg_seconds = (kernel_total_ms / runs) / 1000;
        double bandwidth = 2 * n * sizeof(double) / (kernel_avg_seconds) / 1e9 ;
        
        std::cout << "N=" << n
                    << " CPU=" << cpu_avg << "ms"
                    << " GPU(total)=" << gpu_avg << "ms"
                    << " Kernel=" << kernel_avg_seconds << "s"
                    << " Bandwidth=" << bandwidth << "GB/s\n";

        // Cleanup events
        cudaEventDestroy(start);
        cudaEventDestroy(stop);
    }
}

int main() {

    // std::cout << "Running Naive" << "\n";
    // bench_reduction(naiveReductionKernel); // naive
    // std::cout << "Done Naive" << "\n";
    
    // std::cout << "\n";
    
    // bench_reduction(sequentialAddressingReductionKernel); // sequential addressing
    // std::cout << "Done Sequential" << "\n";
    
    // std::cout << "\n";
    
    std::cout << "Starting Block Sweep" << "\n";
    for (size_t threadsPerBlock = 32; threadsPerBlock <= 1024; threadsPerBlock *= 2) {
        std::cout << "Starting WarpShuffle with \n=== Block Size: " << threadsPerBlock << " ===\n";
        bench_reduction(warpShuffleReductionKernel, threadsPerBlock);
        std::cout << "Done WarpShuffle" << "\n";
    }
    std::cout << "Ending Block Sweep" << "\n";
    
}