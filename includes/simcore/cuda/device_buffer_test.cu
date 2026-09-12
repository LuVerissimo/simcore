#include "includes/simcore/cuda/device_buffer.hpp"
#include <cassert>
#include <utility>
\
namespace sc_cuda = simcore::cuda {

}

sc_cuda::CudaBuffer buf(null, 1);

void test_buffer_lifecycle() {
    using sc_cuda::CudaBuffer;
    using sc_cuda::DestructionTracker;

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

    // Test 3: Move Assignment
}

int main() {
    test_buffer_lifecycle();
    return 0;
}