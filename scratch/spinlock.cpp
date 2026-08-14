#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

class Spinlock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock()   { while (flag.test_and_set(std::memory_order_acquire)) {} }
    void unlock() { flag.clear(std::memory_order_release); }
};

const int N = 1'000'000;
int counter = 0;

// Test with spinlock
Spinlock spin;
void work_spin() {
    for (int i = 0; i < N; ++i) {
        spin.lock();
        counter++;
        spin.unlock();
    }
}

// Test with mutex for comparison
std::mutex mtx;
void work_mutex() {
    for (int i = 0; i < N; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        counter++;
    }
}

int main() {
    // Spinlock
    counter = 0;
    auto t0 = std::chrono::high_resolution_clock::now();
    std::jthread s1(work_spin), s2(work_spin);
    s1.join(); s2.join();
    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "Spinlock:  " << counter << " in "
              << std::chrono::duration<double,std::milli>(t1-t0).count() << " ms\n";

    // Mutex
    counter = 0;
    auto t2 = std::chrono::high_resolution_clock::now();
    std::jthread m1(work_mutex), m2(work_mutex);
    m1.join(); m2.join();
    auto t3 = std::chrono::high_resolution_clock::now();

    std::cout << "Mutex:     " << counter << " in "
              << std::chrono::duration<double,std::milli>(t3-t2).count() << " ms\n";
}