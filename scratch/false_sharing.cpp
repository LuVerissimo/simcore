#include <iostream>
#include <thread>
#include <chrono>

const int N = 100'000'000;

// No padding — both counters on same cache line
struct NoPad {
    double a = 0;
    double b = 0;
};

// Padded — each counter on its own cache line
struct alignas(64) Padded {
    double value = 0;
};

int main() {
    // Test 1: false sharing
    NoPad shared;
    auto t0 = std::chrono::high_resolution_clock::now();
    std::jthread t1([&] { for (int i = 0; i < N; ++i) shared.a += 1.0; });
    std::jthread t2([&] { for (int i = 0; i < N; ++i) shared.b += 1.0; });
    t1.join(); t2.join();
    auto t1_end = std::chrono::high_resolution_clock::now();

    // Test 2: no false sharing
    Padded pa, pb;
    auto t2_start = std::chrono::high_resolution_clock::now();
    std::jthread t3([&] { for (int i = 0; i < N; ++i) pa.value += 1.0; });
    std::jthread t4([&] { for (int i = 0; i < N; ++i) pb.value += 1.0; });
    t3.join(); t4.join();
    auto t2_end = std::chrono::high_resolution_clock::now();

    auto ms = [](auto a, auto b) {
        return std::chrono::duration<double, std::milli>(b - a).count();
    };

    std::cout << "False sharing:    " << ms(t0, t1_end) << " ms\n";
    std::cout << "Padded (no FS):   " << ms(t2_start, t2_end) << " ms\n";
}