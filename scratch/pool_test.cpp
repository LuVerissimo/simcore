#include "math/thread_pool.hpp"
#include <future>
#include <vector>
#include <iostream>

int main() {
    ThreadPool pool(4);
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.submit([i] {
            return i * i;
        }));
    }

    for (auto& f : futures)
        std::cout << f.get() << " ";
    std::cout << "\n";
}