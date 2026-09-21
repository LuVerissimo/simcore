#include "simcore/cuda/vec3.cuh"
#include <cassert>
#include <vector>
#include <iostream>

void test_vec3_add(int n){
    using simcore::cuda::CudaBuffer;
    
    std::vector<vec3f> h_A(n, vec3f{1.0f, 2.0f, 3.0f});
    std::vector<vec3f> h_B(n, vec3f{4.0f, 5.0f, 6.0f});
    std::vector<vec3f> h_C(n);   

    CudaBuffer<vec3f> d_A(n);
    CudaBuffer<vec3f> d_B(n);
    CudaBuffer<vec3f> d_C(n);   
    int size = n * sizeof(vec3f);
    
    cudaMemcpy(d_A.data(), h_A.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B.data(), h_B.data(), size, cudaMemcpyHostToDevice);
    
    simcore::cuda::vec3_add(d_A, d_B, d_C, n);
    
    cudaMemcpy(h_C.data(), d_C.data(), size, cudaMemcpyDeviceToHost);

    for (int i = 0; i < n; i++) {
        assert(h_A[i].x == 5.0f);
        assert(h_B[i].y == 7.0f);
        assert(h_C[i].z == 9.0f);
    }

    std::cout << "vec3_add test passed.\n";
}

int main() {
    
    test_vec3_add(1024);
    return 0;
}