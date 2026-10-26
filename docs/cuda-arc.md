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
    ./lambda-ssh.sh <ip-address>
    pip install cmake
    git clone <repo>
    cd simcore
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


## Week 17: Parallel Reduction (Three Variants)
Read: Kirk & Hwu Ch 6 (performance), Ch 8 (reduction patterns), Ch 10 (reduction optimizations).

Three reduction kernels for GPU dot products, all behind a generic `launch_reduction` that takes a kernel function pointer to eliminate wrapper duplication.

**Naive (interleaved addressing):** stride doubles each step, thread tid % (2*stride) == 0 participates. Causes warp divergence since active threads are non-contiguous.

**Sequential addressing:** stride halves from blockDim.x/2, threads 0..stride-1 participate. Contiguous active threads, no divergence within a warp until stride < 32.

**Warp shuffle:** `__shfl_down_sync` replaces shared memory for intra-warp reduction. Cross-warp partial sums still go through shared memory, then warp 0 does a final shuffle pass.

Benchmark (A100 40GB, kernel-only, 5-run average):

| Variant | 10M (GB/s) | 100M (GB/s) | vs. Naive |
|---------|-----------|------------|-----------|
| Naive | 353 | 769 | 1.0x |
| Sequential | 466 | 1178 | 1.53x |
| Warp Shuffle | 473 | 1099 | 1.43x |

Sequential beats shuffle at this block size (256) because without first-add-during-load coarsening, both do the same total work; sequential's simpler instruction mix wins. Peak bandwidth utilization: 74% of A100 theoretical (~1555 GB/s).

All three pass compute-sanitizer with 0 errors. Not yet wired into CG solver; full GPU-resident CG (r, p, x, Ap kept on device) is prerequisite before reduction integration is useful.


## Week 18: GPU-Resident CG & Kernel Tuning

### GPU-Resident CG Refactor
- Eliminated per-iteration host transfers in `solve_cg_gpu`
- All vectors (`x`, `r`, `p`, `Ap`, `b`) as `CudaBuffer<double>` on device
- Single upload at start, single download of solution at end
- Three-operand `daxpy` kernel: `z[i] = a * x[i] + y[i]` (raw pointer signature)
- Dot products via `launch_reduction` device-pointer overload
- Bug found: p-update had `r` and `p` swapped in daxpy arguments, causing solver stall (beta ~1.0 every iteration, residual not decreasing)

### Block Size Sweep (Warp Shuffle Reduction, N=100M, A100)

| Block Size | Bandwidth (GB/s) | % Peak (2039) |
|-----------|------------------|---------------|
| 32        | 193              | 9.5%          |
| 64        | 377              | 18.5%         |
| 128       | 713              | 35.0%         |
| 256       | 1433             | 70.3%         |
| 512       | 1469             | 72.0%         |
| 1024      | 1469             | 72.0%         |

Knee at 256 threads/block. Below that, too few warps per block to hide memory latency. 256+ gives 8+ warps for the scheduler; no gain beyond that.

### Occupancy vs Bandwidth Plot
*(TODO)*

### First ncu Profiling Session
*(TODO)*

### Reading
- Kirk & Hwu Ch 12 (Merge Sort) and Ch 13 (Radix Sort) — completed