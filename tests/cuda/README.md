# CUDA Arc (Weeks 15-18)

## Environment

WSL2 compiles, Lambda runs.

**WSL2 (no GPU, compile only)**
- CUDA toolkit 12.8 installed, but bare `nvcc` fails due to glibc header conflict (`cospi`/`sinpi`/`rsqrt` noexcept mismatch)
- All CUDA compilation goes through Docker: `nvidia/cuda:12.8.0-devel-ubuntu22.04`
- CPU-only build works natively

**Lambda Labs (A100 40GB, runtime + profiling)**
- On-demand instance, terminate when idle
- Lambda Stack 22.04, driver 580, nvcc 12.8, gcc 11.4
- `ncu` requires `sudo` (`RmProfilingAdminOnly: 1`)
- `nsight-systems` must be installed via apt on each instance

## Build

CPU-only (native):
```
cmake -B build -DSIMCORE_CUDA=OFF && cmake --build build
```

CUDA (Docker on WSL2):
```
./cuda-build.sh
```

CUDA (Lambda, native):
```
pip install cmake
cmake -B build-cuda -DSIMCORE_CUDA=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-cuda
```

## Tests

```
./build-cuda/cuda_test_device_buffer
./build-cuda/cuda_test_saxpy
./build-cuda/cuda_test_vec3
compute-sanitizer --tool memcheck ./build-cuda/cuda_test_saxpy
```

## Week 15 Results

RAII `CudaBuffer<T>`: move-only, noexcept destructor, best-effort release on sticky errors.

SAXPY benchmark (A100 40GB, 5-run average):

| N | CPU (ms) | GPU total (ms) | Kernel (ms) | Transfer (ms) | Speedup (total) | Speedup (kernel) |
|---|----------|-----------------|-------------|----------------|------------------|-------------------|
| 1M | 0.16 | 0.16 | 0.06 | 0.10 | 1.0x | 2.9x |
| 10M | 2.80 | 1.42 | 0.15 | 1.27 | 2.0x | 18.8x |
| 100M | 28.39 | 14.02 | 0.93 | 13.09 | 2.0x | 30.6x |

Key finding: PCIe transfer is 93% of GPU wall time at 100M elements. SAXPY has ~0.17 FLOPs/byte, making it memory-bound. Offloading only wins when data is already on device or kernel arithmetic intensity is higher.

## Workflow

1. Edit on WSL2
2. `./cuda-build.sh` to compile
3. Push to `cuda-arc` branch
4. Launch Lambda, `git pull`, build natively, run/profile
5. Terminate instance