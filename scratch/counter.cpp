#include <iostream>
#include <mutex>
#include <thread>

int shared_counter = 0;
std::mutex mtx;

void func() {
    for (int i = 0; i < 1'000'000; ++i) {
        // std::lock_guard<std::mutex> lock(mtx);
        shared_counter++;
    }
}

int main() {

    std::thread t1(func);
    std::thread t2(func);
    
    t1.join();    
    t2.join(); 

    std::cout << "Final Count (Mutex): " << shared_counter << std::endl;
    return 0;
}