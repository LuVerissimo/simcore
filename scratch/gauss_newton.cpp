
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
double model(double x, double a, double b, double c) {
    return a * std::exp(-b * x) + c;
}

int main() {
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

    double a = 1.0, b = 1.0, c = 0.0;
    for (int iter = 0; iter < 20; ++iter) {

    }

    std::cout << "True:          " << a_true << " " << b_true << " " << c_true << "\n";
    std::cout << "Estimated:          " << a << " " << b << " " << c << "\n";
}