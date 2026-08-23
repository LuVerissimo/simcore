
#include <chrono>
#include <iostream>
#include <memory_resource>
#include <ratio>
#include <vector>
struct Contact {
    double px, py, pz;
    double nx, ny, nz;
    double depth;
    int body_a, body_b;
};

const int N = 1'000'000;
const int FRAMES = 100;

int main() {
    auto t0 = std::chrono::high_resolution_clock::now();
    
    // version 1 new/delete each frame
    for (int f = 0; f < FRAMES; ++f) {
        std::vector<Contact> contacts;
        for (int i = 0; i < N; ++i) {
            contacts.push_back({1,2,3,0,1,0,0.01,i,i+1});
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    
    
    //v2 pmr arena, reset eacg frane
    std::vector<char> buffer(N * sizeof(Contact) + 1024);
    std::pmr::monotonic_buffer_resource pool(buffer.data(), buffer.size());
    auto t2 = std::chrono::high_resolution_clock::now();
    
    for (int f = 0; f < FRAMES; ++f) {
        std::pmr::vector<Contact> contacts(&pool);
        contacts.reserve(N);
        for (int i = 0; i < N; ++i) {
            contacts.push_back({1,2,3,0,1,0,0.01,i,i+1});
        }
        pool.release();
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    auto ms = [](auto a, auto b) {
        return std::chrono::duration<double, std::milli>(b-a).count();
    };

    std::cout << "new/delete:  " << ms(t0, t1) << " ms\n";
    std::cout << "pmr arena:  " << ms(t2, t3) << " ms\n";
}