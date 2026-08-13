#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::queue<int> buffer;
const int MAX_SIZE = 10;
std::mutex mtx;
std::condition_variable cv_full;
std::condition_variable cv_empty;
bool done = false;

void producer() {
    for (int i = 0; i < 20; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_full.wait(lock, [] { return buffer.size() < MAX_SIZE; });
        buffer.push(i);
        std::cout << "Produced: " << i << " (size: " << buffer.size() << ")\n";
        cv_empty.notify_one();
    }
    std::lock_guard<std::mutex> lock(mtx);
    done = true;
    cv_empty.notify_one();
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_empty.wait(lock, [] { return !buffer.empty() || done; });

        if (buffer.empty() && done) break;

        std::cout << "Consumed: " << buffer.front() << "\n";
        buffer.pop();
        cv_full.notify_one(); 
    }
}

int main() {
    std::jthread t1(producer);
    std::jthread t2(consumer);
}