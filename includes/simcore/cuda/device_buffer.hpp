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
    size_t count_;
    DestructionTracker tracker;
public:
    T* data() noexcept { return d_ptr; }
    //Constructor calls cudaMalloc. Throws on failure
    CudaBuffer(size_t elements): d_ptr(nullptr), count_(elements) {
        cudaError_t err = cudaMalloc(&d_ptr, elements * sizeof(T));
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
        }
        d_ptr = nullptr;
        tracker.track_dealloc();
    }

    //Move constructor and move assignment transfer ownership, null out the source.
    CudaBuffer(CudaBuffer&& other) noexcept: d_ptr(other.d_ptr), count_(other.count_), tracker(other.tracker) {
        other.d_ptr = nullptr;
        other.count_ = 0;
    }
    CudaBuffer& operator=(CudaBuffer&& other) noexcept {
        if (this != &other) {

            if (d_ptr != nullptr) {
                cudaError_t err = cudaFree(d_ptr);
                tracker.track_dealloc()

                if (err != cudaSuccess) {
                    std::cerr << "Critical: cudaFree failed in destructor. Error: " << cudaGetErrorString(err) << std::endl;
                }
            }

            d_ptr = other.d_ptr;
            count_ = other.count_;
            tracker = other.tracker;

            other.d_ptr = nullptr;
            other.count_ = 0;
        }

        return *this;
    }

    //Copy constructor and copy assignment are deleted
    CudaBuffer(const CudaBuffer&) = delete;
    CudaBuffer& operator=(const CudaBuffer&) = delete;

    size_t size() const noexcept {
        return count_;
    }
};

}