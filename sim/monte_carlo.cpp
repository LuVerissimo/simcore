
#include "math/vec3.hpp"
#include <functional>
#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>

// 2-link arm FK
struct Vec2 { double x, y; };

Vec2 fk(double t1, double t2, double L1 = 1.0, double L2 = 1.0) {
    double t12 = t1 + t2;
    return {L1*std::cos(t1) + L2*std::cos(t12),
            L1*std::sin(t1) + L2*std::sin(t12)};
}

// Accumulator - online covariance
struct Stats {
    double sum_x = 0, sum_y = 0;
    double sum_xx = 0, sum_yy = 0, sum_xy = 0;
    int count = 0;

    void add(Vec2 p) {
        sum_x += p.x; sum_y += p.y;
        sum_xx += p.x*p.x; sum_yy += p.y*p.y;
        sum_xy += p.x*p.y;
        count++;
    }

    void merge(const Stats& other) {
        sum_x += other.sum_x; sum_y += other.sum_y;
        sum_xx += other.sum_xx; sum_yy += other.sum_yy;
        sum_xy += other.sum_xy;
        count += other.count;
    }

    Vec2 mean() const { return {sum_x/count, sum_y/count}; }

    // Cov = E[xxᵀ] - μμᵀ
    void print_cov() const {
        auto m = mean();
        double cxx = sum_xx/count - m.x*m.x;
        double cyy = sum_yy/count - m.y*m.y;
        double cxy = sum_xy/count - m.x*m.y;
        std::cout << "Cov: [[" << cxx << ", " << cxy << "], ["
                  << cxy << ", " << cyy << "]]\n";
    }
};


std::mutex mtx;

int main() {
    const int N = 10'000'000;
    double mu1 = 0.5, mu2 = 0.8;     // mean joint angles
    double sigma = 0.5;              // input noise std dev

    // --- Single threaded (baseline) ---
    Stats baseline;
    auto t0_start = std::chrono::high_resolution_clock::now();
    
    std::mt19937 gen0(42);
    std::normal_distribution<double> d1(mu1, sigma), d2(mu2, sigma);
    for (int i = 0; i < N; ++i) baseline.add(fk(d1(gen0), d2(gen0)));

    auto t0_end = std::chrono::high_resolution_clock::now();

    
    // --- Version A: shared accumulator with mutex ---
    Stats shared_stats;
    
    
    auto work_a = [&](int seed, int count) {
        std::mt19937 gen(seed);
        std::normal_distribution<double> d1(mu1, sigma), d2(mu2, sigma);
        for (int i = 0; i < count; ++i) {
            std::lock_guard<std::mutex> lock(mtx);
            shared_stats.add(fk(d1(gen), d2(gen)));
        }
    };
    auto t1_start = std::chrono::high_resolution_clock::now();
    std::jthread a1(work_a, 1, N/2);
    std::jthread a2(work_a, 2, N/2);
    a1.join(); a2.join();
    auto t1_end = std::chrono::high_resolution_clock::now();


    
    // --- Version B: per-thread accumulators, merged at end ---
    Stats local1, local2;
    
    
    auto work_b = [&](Stats& local, int seed, int count) {
        std::mt19937 gen(seed);
        std::normal_distribution<double> d1(mu1, sigma), d2(mu2, sigma);
        for (int i = 0; i < count; ++i) {
            local.add(fk(d1(gen), d2(gen)));
        }
    };
    
    auto t2_start = std::chrono::high_resolution_clock::now();
    std::jthread b1(work_b, std::ref(local1), 1, N/2);
    std::jthread b2(work_b, std::ref(local2), 2, N/2);
    b1.join(); b2.join();
    auto t2_end = std::chrono::high_resolution_clock::now();

    Stats merged;
    merged.merge(local1);
    merged.merge(local2);

    
    auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count();};
        
        
    std::cout << "Single Threaded:    " << ms(t0_start, t0_end) << " ms\n";
    std::cout << "Version A:    " << ms(t1_start, t1_end) << " ms\n";
    std::cout << "Version B:    " << ms(t2_start, t2_end) << " ms\n";
    int total = baseline.count + shared_stats.count + merged.count;
    std::cout << "Total sum:    " << total << "\n";



    // Empirical
    auto m = merged.mean();
    std::cout << "Mean: " << m.x << ", " << m.y << "\n";
    merged.print_cov();
}