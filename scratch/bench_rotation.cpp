#include "math/vec3.hpp"
#include "math/mat3.hpp"
#include "math/quat.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>

int main() {
    const int N = 1'000'000;
    const double angle = 0.001;  // small rotation per step
    quat q = from_axis_angle({0, 0, 1}, angle);

    // --- Quaternion chain ---
    quat accum_q = {1, 0, 0, 0};

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        accum_q = accum_q * q;
    auto t1 = std::chrono::high_resolution_clock::now();

// --- Matrix chain ---
    mat3 m = rotation_z(angle); 
    mat3 accum_m = mat3::identity();

    auto m0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
        accum_m = accum_m * m;
    auto m1 = std::chrono::high_resolution_clock::now();
    // --- Print ---
    // times in ms, and: norm(accum_q) after 1M multiplies

    auto quat_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    auto mat3_ms = std::chrono::duration<double, std::milli>(m1 - m0).count();

    std::cout << "Quat: " << quat_ms << " ms, norm drift: " << norm(accum_q) << "\n";
    std::cout << std::setprecision(17) << norm(accum_q) << "\n";
    std::cout << "Mat3: " << mat3_ms << " ms, trace: " << accum_m(0,0) + accum_m(1,1) + accum_m(2,2) << "\n";
}