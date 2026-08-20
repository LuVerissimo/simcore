
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

// Model: y = a * exp(-b*x) + c
double model(double x, double a, double b, double c) {
    return a * std::exp(-b * x) + c;
}

int main() {
    // ground truth
    double a_true = 5.0, b_true = 0.3, c_true = 1.0;

    // noisy data
    const int M = 100; // data point
    std::mt19937 gen(42);
    std::normal_distribution<double> noise(0, 0.1);

    std::vector<double> x_data(M), y_data(M);
    for (int i = 0; i < M; ++i) {
        x_data[i] = i * 0.1;
        y_data[i] = model(x_data[i], a_true, b_true, c_true) + noise(gen);
    }

    // Initial wrong guess
    double a = 1.0, b = 1.0, c = 0.0;

    for (int iter = 0; iter < 20; ++iter) {
        // J jacobian mat3
        // JᵀJ (3×3) and Jᵀr (3×1)
        // JᵀJ δ = -Jᵀr
        double JtJ[3][3] = {};
        double Jtr[3] = {};
        double cost = 0;

        for (auto i = 0; i < M; ++i) {
            //residual function
            double e = std::exp(-b * x_data[i]);
            double r_i = y_data[i] - (a * e + c);

            cost += r_i * r_i;

            double Ji[3] = {-e, a * x_data[i] * e, -1.0};

            for (int p = 0; p < 3; ++p) {
                Jtr[p] += Ji[p] * r_i; 

                for (int q = 0; q < 3; ++q) {
                    JtJ[p][q] += Ji[p] * Ji[q];
                }
            }
        }
        std::cout << "Iter " << iter << " cost: " << 0.5 * cost << "\n";
        // update a += δ[0], b += δ[1], c += δ[2]
        // cost = ½‖r‖²
    }

    std::cout << "True:          " << a_true << " " << b_true << " " << c_true << "\n";
    std::cout << "Estimated:          " << a << " " << b << " " << c << "\n";
}