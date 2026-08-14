#pragma once
#include <array>
#include <atomic>

template<typename T, int N>
class SPSCQueue {
    std::array<T, N> buf;
    alignas(64) std::atomic<int> head{0};  // producer writes
    alignas(64) std::atomic<int> tail{0};  // consumer writes

public:
    bool push(const T& val) {
        // Your work:
        // 1. Load head (relaxed — only producer writes it)
        auto curr = head.load(std::memory_order_relaxed);
        // 2. Compute next = (head + 1) % N
        auto next = (curr + 1) % N;
        // 3. If next == tail (acquire) → full, return false
        auto prev_tail = tail.load(std::memory_order_acquire);
        if (next == prev_tail) {
            return false;
        } 
        // 4. Write to buf[head]
        buf[curr] = val;
        // 5. Store next to head (release — makes the write visible)
        head.store(next, std::memory_order_release);
        // 6. Return true
        return true;
    }

    bool pop(T& val) {
        // Your work:
        // 1. Load tail (relaxed — only consumer writes it)
        auto prev_tail = tail.load(std::memory_order_relaxed);
        auto curr_head = head.load(std::memory_order_acquire);
        // 2. If tail == head (acquire) → empty, return false
        if (prev_tail == curr_head) {
            return false;
        }
        // 3. Read from buf[tail]
        val = buf[prev_tail];
        // 4. Store (tail + 1) % N to tail (release)
        auto next = (prev_tail + 1) % N;
        tail.store(next, std::memory_order_release);
        // 5. Return true
        return true;
    }
};