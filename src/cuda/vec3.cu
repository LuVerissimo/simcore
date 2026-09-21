#include "simcore/cuda/vec3.cuh"
#include <type_traits>
#include <stdexcept>
#include <string>

static_assert(std::is_trivially_copyable_v<vec3f>,
    "vec3f must be trivially copyable for cudaMemcpy");

__global__
void vecAddKernel(const vec3f* A, const vec3f* B, vec3f* C, int n) {
    int i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i < n) {
        C[i] = A[i] + B[i];
    }
}

namespace simcore::cuda {
void vec3_add(CudaBuffer<vec3f>& A, CudaBuffer<vec3f>& B, CudaBuffer<vec3f>& C, int n) {
    int threadsPerBlock = 256;
    int blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;
    vecAddKernel<<<blocksPerGrid, threadsPerBlock>>>(A.data(), B.data(), C.data(), n);

    cudaError_t errSync = cudaDeviceSynchronize();
    if (errSync != cudaSuccess) {
        throw std::runtime_error("Kernel Launch failed: " + std::string(cudaGetErrorString(errSync)));
    }
}}
