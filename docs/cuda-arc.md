# CUDA Arc (Weeks 15-18)

## Environment

Two machines. WSL2 compiles, Lambda runs.

**WSL2 (no GPU, compile only)**
- CUDA toolkit 12.8, bare nvcc fails due to glibc header conflict (cospi/sinpi/rsqrt noexcept mismatch)
- All CUDA compilation via Docker: nvidia/cuda:12.8.0-devel-ubuntu22.04
- CPU-only build works natively

**Lambda Labs (A100 40GB, runtime + profiling)**
- On-demand instance, terminate when idle
- Lambda Stack 22.04, driver 580, nvcc 12.8, gcc 11.4
- ncu requires sudo (RmProfilingAdminOnly: 1)
- nsight-systems installed via apt on each instance

## Build

CPU-only (native):

    cmake -B build -DSIMCORE_CUDA=OFF && cmake --build build

CUDA (Docker on WSL2):

    ./cuda-build.sh

CUDA (Lambda):

    pip install cmake
    cmake -B build-cuda -DSIMCORE_CUDA=ON -DCMAKE_BUILD_TYPE=Release
    cmake --build build-cuda

## Tests

    ./build-cuda/cuda_test_device_buffer
    ./build-cuda/cuda_test_saxpy
    ./build-cuda/cuda_test_vec3
    ./build-cuda/test_cg_gpu
    compute-sanitizer --tool memcheck ./build-cuda/cuda_test_saxpy
    compute-sanitizer --tool memcheck ./build-cuda/test_cg_gpu

## Workflow

1. Edit on WSL2
2. ./cuda-build.sh to compile
3. Push to cuda-arc branch
4. Launch Lambda, git pull, build natively, run/profile
5. Terminate instance

## Week 15: Environment + First Kernels

RAII CudaBuffer<T>: move-only, noexcept destructor, best-effort release on sticky CUDA errors. cudaFree can return errors from earlier kernel faults (sticky errors); destructor swallows them and logs.

SAXPY benchmark (A100 40GB, 5-run average):

| N | CPU (ms) | GPU total (ms) | Kernel (ms) | Transfer (ms) | Speedup (total) | Speedup (kernel) |
|---|----------|-----------------|-------------|---------------|-----------------|------------------|
| 1M | 0.16 | 0.16 | 0.06 | 0.10 | 1.0x | 2.9x |
| 10M | 2.80 | 1.42 | 0.15 | 1.27 | 2.0x | 18.8x |
| 100M | 28.39 | 14.02 | 0.93 | 13.09 | 2.0x | 30.6x |

Key finding: PCIe transfer is 93% of GPU wall time at 100M elements. SAXPY has ~0.17 FLOPs/byte. Offloading only wins when data is already on device or arithmetic intensity is higher.

## Week 16: CSR SpMV + GPU CG Solver

Naive CSR SpMV kernel: one thread per row, each thread walks its row's nonzeros.

Standalone SpMV benchmark (1M rows, ~5 nnz/row, 2D Laplace):

| Metric | Value |
|--------|-------|
| CPU | 3.78 ms |
| GPU (total) | 16.81 ms |
| Kernel only | 0.11 ms |
| Speedup (total) | 0.22x |
| Speedup (kernel) | 33.2x |

Standalone SpMV is slower than CPU due to transfer overhead. But inside CG the matrix stays on device across all iterations, paying transfer cost once.

GPU CG solver: SpMV on device, dot products and axpy on CPU (download/upload each iteration). Converges in 225 iterations on 100x100 2D Laplace, matches CPU CG to 1e-10. compute-sanitizer clean.