// #include <iostream>
// #include <ratio>
// #include <vector>
// #include <chrono>




// int main() {
//     const int N = 1'000'000;

//     // wo/reserve
//     auto t0 = std::chrono::high_resolution_clock::now();
//     std::vector<double> v1;
//     for (int i = 0; i < N; ++i) v1.push_back(i);
//     auto t1 = std::chrono::high_resolution_clock::now();

//     // w/reserve
//     auto t2 = std::chrono::high_resolution_clock::now();
//     std::vector<double> v2;
//     v2.reserve(N);
//     for (int i = 0; i < N; ++i) v2.push_back(i);
//     auto t3 = std::chrono::high_resolution_clock::now();


        // push_back during iteration invalidates iterators → segfault.
//     auto t4 = std::chrono::high_resolution_clock::now();
//     std::vector<int> v = {1,2,3};
//     for (auto it = v.begin(); it != v.end(); ++it) v.push_back(*it);
//     auto t5 = std::chrono::high_resolution_clock::now();

//     auto ms1 = std::chrono::duration<double, std::milli>(t1-t0).count();
//     auto ms2 = std::chrono::duration<double, std::milli>(t3-t2).count();
//     auto ms3 = std::chrono::duration<double, std::milli>(t5-t4).count();

//     std::cout << "No reserve: " << ms1 << " ms\n";
//     std::cout << "Reserved: " << ms2 << " ms\n";
//     std::cout << "Invalidation: " << ms3 << " ms\n";
// }

#include <iostream>
#include <vector>
#include <span>

double sum(std::span<double> data) {
    double total = 0;
    for (double d : data) total += d;
    return total;
}

int main() {
    std::vector<double> v = {10, 20, 30, 40, 50};

    std::cout << "All:      " << sum(v) << "\n";                        // whole vector
    std::cout << "Middle 3: " << sum({v.data() + 1, 3}) << "\n";       // subrange: 20,30,40
    std::cout << "Last 2:   " << sum({v.data() + 3, 2}) << "\n";       // subrange: 40,50
}