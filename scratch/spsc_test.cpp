#include "math/spsc_queue.hpp"
#include <iostream>
#include <thread>

int main() {
    SPSCQueue<int, 16> q;

    std::jthread producer([&] {
        for (int i = 0; i < 100; ++i)
            while (!q.push(i)) {}  // spin until room
    });

    std::jthread consumer([&] {
        for (int i = 0; i < 100; ++i) {
            int val;
            while (!q.pop(val)) {}  // spin until data
            std::cout << val << " ";
        }
        std::cout << "\n";
    });
}