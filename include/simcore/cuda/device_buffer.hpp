#pragma once
#include <iostream>
#include <cuda_runtime.h>
#include <stdexcept>
#include <string>

namespace simcore::cuda {

struct DestructionTracker {
    int* active_allocations;

    void track_alloc() { if (active_allocations) (*active_allocations)++;}
    void track_dealloc() { if (active_allocations) (*active_allocations)--;}
};


template <typename T>
class CudaBuffer {
private:
    T* d_ptr;
    size_t buffer_size;
    DestructionTracker tracker;
public:
    T* data() noexcept { return d_ptr; }
    const T* data() const noexcept { return d_ptr; }
    //Constructor calls cudaMalloc. Throws on failure
    CudaBuffer(size_t size, DestructionTracker dt = {}): d_ptr(nullptr), buffer_size(size), tracker(dt) {

        cudaError_t err = cudaMalloc(&d_ptr, size * sizeof(T));
        if (err != cudaSuccess) {
            throw std::runtime_error("CUDA Malloc failed" + std::string(cudaGetErrorString(err)));
        }
        tracker.track_alloc();
    } 

    //Destructor calls cudaFree. noexcept,
    ~CudaBuffer() noexcept {
        if (d_ptr != nullptr) {
            cudaError_t err = cudaFree(d_ptr);

            if (err != cudaSuccess) {
                std::cerr << "Critical: cudaFree failed in destructor. Error: " << cudaGetErrorString(err) << std::endl;
            }
            tracker.track_dealloc();
        }
        d_ptr = nullptr;
    }

    //Move constructor and move assignment transfer ownership, null out the source.
    CudaBuffer(CudaBuffer&& other) noexcept: d_ptr(other.d_ptr), buffer_size(other.buffer_size), tracker(other.tracker) {
        other.d_ptr = nullptr;
        other.buffer_size = 0;
    }
    CudaBuffer& operator=(CudaBuffer&& other) noexcept {
        if (this != &other) {

            if (d_ptr != nullptr) {
                cudaError_t err = cudaFree(d_ptr);
                tracker.track_dealloc();

                if (err != cudaSuccess) {
                    std::cerr << "Critical: cudaFree failed in destructor. Error: " << cudaGetErrorString(err) << std::endl;
                }
            }

            d_ptr = other.d_ptr;
            buffer_size = other.buffer_size;
            tracker = other.tracker;

            other.d_ptr = nullptr;
            other.buffer_size = 0;
        }

        return *this;
    }

    //Copy constructor and copy assignment are deleted
    CudaBuffer(const CudaBuffer&) = delete;
    CudaBuffer& operator=(const CudaBuffer&) = delete;

    size_t size() const noexcept {
        return buffer_size;
    }
};

}