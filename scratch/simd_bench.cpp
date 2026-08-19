#include <iostream>
#include <chrono>
#include <cmath>
#include <immintrin.h>
#include <ratio>

// const int N = 1'000'000;
const int N = 1024;
alignas (32) double a[N], b[N];

double dot_scalar(const double* a, const double* b, int n) {
    double sum = 0;
    for (int i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

double dot_avx(const double* a, const double* b, int n) {
    __m256d sum = _mm256_setzero_pd();

    for (int i = 0; i < n; i += 4) {
        sum = _mm256_fmadd_pd(_mm256_load_pd(a+i), _mm256_load_pd(b+i), sum);
    }
    alignas(32) double tmp[4];
    _mm256_store_pd(tmp, sum);
    return tmp[0] + tmp[1] + tmp[2] + tmp[3];
}


int main() {
    for (int i = 0; i < N; ++i) { a[i] = i * 0.001; b[i] = i * 0.002; }

    const int REPS = 1000;
    auto bench = [](auto fn, const char* label) {
        auto t0 = std::chrono::high_resolution_clock::now();

        double result = 0;
        for (int r = 0; r < REPS; ++r) result = fn(a, b, N);

        auto t1 = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration<double, std::micro>(t1-t0).count() / REPS;

        std::cout << label << ": " << us << " us, result=" << result << "\n";
    };

    bench(dot_scalar, "Scalar");
    bench(dot_avx, "AVX2  ");
}