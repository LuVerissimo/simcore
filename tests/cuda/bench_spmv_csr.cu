#include "simcore/cuda/spmv.cuh"
#include "simcore/cuda/device_buffer.hpp"
#include <chrono>
#include <cuda_runtime.h>
#include <vector>
#include <cassert>
#include <iostream>

auto ms = [](auto a, auto b) {
    return std::chrono::duration<double, std::milli>(b-a).count();
};


void bench_spmv_CSR() {
    using simcore::cuda::CudaBuffer;
    for (int numRows : {1'000'000}) {
        double cpu_total = 0, gpu_total = 0, kernel_total = 0;
        int runs = 5;
        int size = numRows * sizeof(double);

        //Setup host arrays
        std::vector<double> h_x(numRows, 1.0);
        std::vector<double> h_y_cpu(numRows, 2.0);
        std::vector<double> h_y_gpu(numRows, 2.0);

        //generate host CSR matrix structure
        int gridSize = std::sqrt(numRows); 
        CSRMatrix laplaceA = generateLaplace2DCSR(gridSize);
        int nnz = laplaceA.values.size();

        //setup device allocation
        CudaBuffer<double> d_x(numRows);   
        CudaBuffer<double> d_y(numRows);
        CudaBuffer<int> rowsPtrs(numRows + 1);
        CudaBuffer<int> colIdx(nnz);
        CudaBuffer<double> values(nnz);

        for (int r = 0; r < runs; r++) {
            //CPU
            auto t0 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < numRows; i++) {
                double sum = 0.0;
                int row_start = laplaceA.rowPtrs[i];
                int row_end = laplaceA.rowPtrs[i + 1];
                
                for (int j = row_start; j < row_end; ++j) {
                    sum += laplaceA.values[j] * h_x[laplaceA.colIdx[j]];
                }
                h_y_cpu[i] = sum;
            }

            auto t1 = std::chrono::high_resolution_clock::now();
            cpu_total += ms(t0, t1);

            //GPU (total: upload + kernel + download)
            t0 = std::chrono::high_resolution_clock::now();
            
            cudaMemcpy(d_x.data(), h_x.data(), size, cudaMemcpyHostToDevice);
            cudaMemcpy(d_y.data(), h_y_gpu.data(), size, cudaMemcpyHostToDevice);
            
            
            //upload CSR Structures
            cudaMemcpy(rowsPtrs.data(), laplaceA.rowPtrs.data(), (numRows + 1) * sizeof(int), cudaMemcpyHostToDevice);
            cudaMemcpy(colIdx.data(), laplaceA.colIdx.data(), nnz * sizeof(int), cudaMemcpyHostToDevice);
            cudaMemcpy(values.data(), laplaceA.values.data(), nnz * sizeof(double), cudaMemcpyHostToDevice);

            //run Kernel
            simcore::cuda::spmvCSR(rowsPtrs.data(), colIdx.data(), values.data(), d_x.data(), d_y.data(), numRows);

            //download results
            cudaMemcpy(h_y_gpu.data(), d_y.data(), size, cudaMemcpyDeviceToHost);

            t1 = std::chrono::high_resolution_clock::now();
            gpu_total += ms(t0, t1);


            // Kernel only (data already on device from above, re-upload fresh)
            cudaMemcpy(d_y.data(), h_y_gpu.data(), size, cudaMemcpyHostToDevice);
            
            t0 = std::chrono::high_resolution_clock::now();
            simcore::cuda::spmvCSR(rowsPtrs.data(), colIdx.data(), values.data(), d_x.data(), d_y.data(), numRows);

            cudaDeviceSynchronize();

            t1 = std::chrono::high_resolution_clock::now();
            kernel_total += ms(t0, t1);

            for (int i = 0; i < numRows; i++) {
                assert(std::abs(h_y_cpu[i] - h_y_gpu[i]) < 1e-10);
            }
        }
        double cpu_avg = cpu_total / runs;
        double gpu_avg = gpu_total / runs;
        double kernel_avg = kernel_total / runs;
        double transfer_avg = gpu_avg - kernel_avg;

        std::cout << " N=" << numRows
                  << " CPU=" << cpu_avg << "ms"
                  << " GPU(total)=" << gpu_avg << "ms"
                  << " Kernel=" << kernel_avg << "ms"
                  << " Transfer=" << transfer_avg << "ms"
                  << " Speedup(total)=" << cpu_avg / gpu_avg << "x"
                  << " Speedup(kernel)=" << cpu_avg / kernel_avg << "x\n";
    }
}

int main() {
    bench_spmv_CSR();
}