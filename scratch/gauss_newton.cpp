
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <utility>
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
    double a = 3.0, b = 0.5, c = 0.5;

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
        double lambda = 10.0; // damping
        JtJ[0][0] += lambda;
        JtJ[1][1] += lambda;
        JtJ[2][2] += lambda;
        // cost = ½‖r‖²
        std::cout << "Iter " << iter << " cost: " << 0.5 * cost << "\n";

        double rhs[3] = {-Jtr[0], -Jtr[1], -Jtr[2]};

        double A[3][4];
        for (int p = 0; p < 3; ++p) {
            for (int q = 0; q < 3; ++q) A[p][q] = JtJ[p][q];
            A[p][3] = rhs[p];
        }

        for (int col = 0; col < 3; ++col) {
            int best = col;
            for (int row = col + 1; row < 3; ++row) {
                if (std::abs(A[row][col]) > std::abs(A[best][col])) best = row;
            }
            std::swap(A[col], A[best]);
            for (int row = col + 1; row < 3; ++row) {
                double f = A[row][col] / A[col][col];
                for (int j = col; j < 4; ++j) A[row][j] -= f * A[col][j];
            }
        }

        double delta[3];
        for (int i = 2; i >= 0; --i) {
            delta[i] = A[i][3];
            for (int j = i + 1; j < 3; ++j) delta[i] -= A[i][j] * delta[j];
            delta[i] /= A[i][i];
        }

        // update a += δ[0], b += δ[1], c += δ[2]
        a += delta[0]; b += delta[1]; c+= delta[2];

        double norm_d = sqrt(delta[0] * delta[0]  + delta[1]* delta[1]  + delta[2] * delta[2]);
        std::cout << "  params: " << a << " " << b << " " << c << " |delta|: " << norm_d << "\n";

        if (norm_d < 1e-10) break;
    }

    std::cout << "\nTrue:          " << a_true << " " << b_true << " " << c_true << "\n";
    std::cout << "Estimated:          " << a << " " << b << " " << c << "\n";
}