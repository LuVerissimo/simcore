#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <vector>
#include <iomanip>


std::vector<double> heat_2d_flat(int N, double r, int steps) {
    std::vector<double> u_curr(N * N, 0.0);
    std::vector<double> u_next(N * N, 0.0);

    // Heat spike in center
    u_curr[(N/2) * N + (N/2)] = 100.0;

    for (int step = 0; step < steps; ++step) {
        for (int i = 1; i < N - 1; ++i) {
            for (int j = 1; j < N - 1; ++j) {
                int idx = i * N + j;
                u_next[idx] = u_curr[idx] + r * (
                    u_curr[idx - N] + u_curr[idx + N] +
                    u_curr[idx - 1] + u_curr[idx + 1] -
                    4.0 * u_curr[idx]);
            }
        }
        std::swap(u_curr, u_next);  // cheaper than copy
    }

    std::cout << "[Flat] Center after " << steps << " steps: " << u_curr[(N/2)*N + (N/2)] << "\n";
    return u_curr;
}

std::vector<std::vector<double>> heat_2d_naive(int N, double r, int steps) {
    std::vector<std::vector<double>> u_curr(N, std::vector<double>(N,0.0));
    std::vector<std::vector<double>> u_next(N, std::vector<double>(N,0.0));

    u_curr[N/2][N/2] = 100.0;

    for (int step = 0; step < steps; ++step) {
        for (int i = 1; i < N - 1; ++i) {
            for (int j = 1; j < N - 1; ++j) {
                u_next[i][j] =  u_curr[i][j] + r * (
                    u_curr[i-1][j] + u_curr[i+1][j] + 
                    u_curr[i][j-1] + u_curr[i][j+1] - 
                    4.0 * u_curr[i][j]);
            }
        }
        std::swap(u_curr, u_next);
    }

    std::cout << "[Naive] Center after " << steps << " steps: "  << u_curr[N/2][N/2] << "\n";
    return u_next;
}

std::vector<double> heat_2d_tiled(int N, double r, int steps, int B = 32) {
    std::vector<double> u_curr(N * N, 0.0);
    std::vector<double> u_next(N * N, 0.0);

    u_curr[(N/2) * N + (N/2)] = 100.0;
    
    for (int step = 0; step < steps; ++step) {
        for (int bi = 1; bi < N - 1; bi += B)
            for (int bj = 1; bj < N - 1; bj += B)
                for (int i = bi; i < std::min(bi + B, N - 1); ++i)
                    for (int j = bj; j < std::min(bj + B, N - 1); ++j) {
                        int idx = i * N + j;
                        u_next[idx] = u_curr[idx] + r * (
                            u_curr[idx - N] + u_curr[idx + N] +
                            u_curr[idx - 1] + u_curr[idx + 1] -
                            4.0 * u_curr[idx]);
                    }
        std::swap(u_curr, u_next);
    }

    std::cout << "[Tiled] Center after " << N << " steps: "  << u_curr[(N/2)*N + (N/2)] << "\n";
    return u_next;
}

int main() {
    for (int N : {100, 500, 1000}) {
        int steps = 1000;
        double r = 0.24;

        auto t0 = std::chrono::high_resolution_clock::now();
        auto v1 = heat_2d_naive(N, r, steps);
        auto t1 = std::chrono::high_resolution_clock::now();
        auto v2 = heat_2d_flat(N, r, steps);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto v3 = heat_2d_tiled(N, r, steps);
        auto t3 = std::chrono::high_resolution_clock::now();

        auto ms = [](auto a, auto b) {
            return std::chrono::duration<double, std::milli>(b - a).count();
        };

        std::cout << "N=" << N
                  << " naive: " << ms(t0,t1)
                  << " flat: "  << ms(t1,t2)
                  << " tiled: " << ms(t2,t3) << " ms\n";
    }
}