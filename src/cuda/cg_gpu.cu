#include "simcore/cuda/cg_gpu.cuh"
#include "simcore/cuda/spmv.cuh"
#include "simcore/cuda/device_buffer.hpp"
#include "simcore/cuda/reduction.cuh"
#include "simcore/cuda/saxpy.cuh"

using simcore::cuda::CudaBuffer;
using simcore::cuda::spmvCSR;
using simcore::cuda::launch_reduction;
using simcore::cuda::daxpy;

namespace simcore::cuda {
    [[nodiscard]] std::vector<double> solve_cg_gpu(
        const SparseMatrix& A,
        const std::vector<double>& b,
        std::vector<double> x,
        double tol,
        int max_iter
    ) {
        //generate host CSR matrix structure
        int numRows = A.rows;
        int nnz = A.values.size();
        // device buffers
        CudaBuffer<double> d_x(numRows); // solution vector
        CudaBuffer<double> d_r(numRows); // residual
        CudaBuffer<double> d_p(numRows); // search direction
        CudaBuffer<double> d_Ap(numRows); // SpMV result
        CudaBuffer<double> d_b(numRows); // right hand side
        CudaBuffer<int> rowsPtrs(numRows + 1);
        CudaBuffer<int> colIdx(nnz);
        CudaBuffer<double> values(nnz); 
        
        // upload x -> d_p
        cudaMemcpy(d_p.data(), x.data(), numRows * sizeof(double), cudaMemcpyHostToDevice);
        // upload x -> d_x
        cudaMemcpy(d_x.data(), x.data(), numRows * sizeof(double), cudaMemcpyHostToDevice);
        // upload b -> d_b
        cudaMemcpy(d_b.data(), b.data(), numRows * sizeof(double), cudaMemcpyHostToDevice);

        // upload inorder to use kernel
        cudaMemcpy(rowsPtrs.data(), A.row_ptr.data(), (numRows + 1) * sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy(colIdx.data(), A.col_idx.data(), nnz * sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy(values.data(), A.values.data(), nnz * sizeof(double), cudaMemcpyHostToDevice);

        // initial kernel
        spmvCSR(rowsPtrs.data(), colIdx.data(), values.data(), d_p.data(), d_Ap.data(), numRows); // to get Ax

        // calculate residual
        daxpy(d_Ap.data(), d_b.data(), d_r.data(), -1.0, numRows); // to get r=b-Ax
        cudaMemcpy(d_p.data(), d_r.data(), numRows * sizeof(double), cudaMemcpyDeviceToDevice);

        for (auto k = 0; k < max_iter; ++k) { 
            //run kernel
            spmvCSR(rowsPtrs.data(), colIdx.data(), values.data(), d_p.data(), d_Ap.data(), numRows);
            auto rr = launch_reduction(d_r.data(), d_r.data(), numRows, warpShuffleReductionKernel);
            if (k == 0) std::cout << "rr @ 0: " << rr << "\n";
            auto pAp = launch_reduction(d_p.data(), d_Ap.data(), numRows, warpShuffleReductionKernel);

            auto alpha = rr / pAp;
            std::cout << "alpha: " << alpha << "\n";
            
            daxpy(d_p.data(), d_x.data(), d_x.data(), alpha, numRows); // x = x + alpha * p
            daxpy(d_Ap.data(), d_r.data(), d_r.data(), -alpha, numRows); // r = r - alpha * Ap
            
            auto rr_new = launch_reduction(d_r.data(), d_r.data(), numRows, warpShuffleReductionKernel);
            std::cout << "CG iterations: " << k << "\n";
            if (sqrt(rr_new) < tol) break;
            
            
            auto beta = rr_new / rr;
            std::cout << "beta: " << beta << "\n";
            daxpy(d_r.data(), d_p.data(), d_p.data(), beta, numRows); // p = r + beta * p
        }
        cudaMemcpy(x.data(), d_x.data(), numRows * sizeof(double), cudaMemcpyDeviceToHost);
        return x;
    }
}