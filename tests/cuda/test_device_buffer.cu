#include "simcore/cuda/device_buffer.hpp"
#include <cassert>
#include <utility>
#include <iostream>

void test_buffer_lifecycle() {
    using simcore::cuda::CudaBuffer;
    using simcore::cuda::DestructionTracker;

    int active_buffers = 0;
    DestructionTracker tracker{&active_buffers};

    std::cout << "Running CudaBuffer Tests. \n";

    // Test 1: Construction and Destruction Lifecycle 
    {
        CudaBuffer<float> buf(1024, tracker);
        assert(buf.size() == 1024);
        assert(active_buffers == 1);
    }
    assert(active_buffers == 0);
    std::cout << "-> Step 1: Construct and Destruct pass. \n";
    
    // Test 2: Move Construction
    {
        CudaBuffer<float> buf(512, tracker);
        assert(active_buffers == 1);
        float* ptr = buf.data();

        CudaBuffer<float> moved_buf(std::move(buf));

        assert(moved_buf.size() == 512);
        assert(moved_buf.data() == ptr);
        assert(buf.data() == nullptr);
        assert(buf.size() == 0);

        assert(active_buffers == 1);
    }
    assert(active_buffers == 0);
    std::cout << "-> Step 2: Move Constructor pass. \n";

    // Test 3: Move Assignment
    {
        CudaBuffer<float> buf_A(100, tracker);
        CudaBuffer<float> buf_B(200, tracker);
        assert(active_buffers == 2);

        float* ptr_B = buf_B.data();

        buf_A = std::move(buf_B);
        assert(active_buffers == 1);
        assert(buf_A.size() == 200);
        assert(buf_A.data() == ptr_B);
        assert(buf_B.data() == nullptr);

    }
    assert(active_buffers == 0);
    std::cout << "-> Step 3: Move assignment pass. \n";

    std::cout << "Lifecycle tests passed. \n";
}

int main() {
    test_buffer_lifecycle();
    return 0;
}