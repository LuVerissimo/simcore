#include "simcore/cuda/cg_gpu.cuh"
#include "simcore/cuda/spmv.cuh"
#include "simcore/cuda/device_buffer.hpp"

using simcore::cuda::CudaBuffer;
using simcore::cuda::spmvCSR;

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
        // device 
        CudaBuffer<int> rowsPtrs(numRows + 1);
        CudaBuffer<int> colIdx(nnz);
        CudaBuffer<double> values(nnz);
        
        // device 
        CudaBuffer<double> d_Ap(numRows);   
        CudaBuffer<double> d_p(numRows);

        // upload inorder to use kernel
        cudaMemcpy(rowsPtrs.data(), A.row_ptr.data(), (numRows + 1) * sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy(colIdx.data(), A.col_idx.data(), nnz * sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy(values.data(), A.values.data(), nnz * sizeof(double), cudaMemcpyHostToDevice);

        // upload x -> d_p
        cudaMemcpy(d_p.data(), x.data(), numRows * sizeof(double), cudaMemcpyHostToDevice);

        // initial kernel
        spmvCSR(rowsPtrs.data(), colIdx.data(), values.data(), d_p.data(), d_Ap.data(), numRows);

        // initial download
        std::vector<double> Ax(numRows);
        cudaMemcpy(Ax.data(), d_Ap.data(), numRows * sizeof(double), cudaMemcpyDeviceToHost);

        // calculate residual
        auto r = axpy(-1.0, Ax, b);
        auto p = r;

        for (auto k = 0; k < max_iter; ++k) { 
            //upload
            cudaMemcpy(d_p.data(), p.data(), numRows * sizeof(double), cudaMemcpyHostToDevice);

            //run kernel
            spmvCSR(rowsPtrs.data(), colIdx.data(), values.data(), d_p.data(), d_Ap.data(), numRows);
            
            //download a_Ap to host
            std::vector<double> Ap(numRows);
            cudaMemcpy(Ap.data(), d_Ap.data(), numRows * sizeof(double), cudaMemcpyDeviceToHost);

            
            //CPU dot/axpy from before
            double rr = dot(r, r);
            auto alpha = rr / dot(p, Ap);
            x = axpy(alpha, p, x);
            auto r_new = axpy(-alpha, Ap, r);

            std::cout << "CG iterations: " << k << "\n";
            if (sqrt(dot(r_new,r_new)) < tol) break;

            auto beta = dot(r_new,r_new) / rr;
            p = axpy(beta, p,r_new);
            r = r_new;
        }
        return x;
    }
}