#include "math/matx.hpp"
#include "math/differentiation.hpp"
#include "math/vec3.hpp"
#include <iostream>
#include <cmath>

// Forward kinematics: (θ₁, θ₂) → (px, py)
std::vector<double> fk(const std::vector<double>& theta) {
    double L1 = 1.0, L2 = 1.0;
    double t1 = theta[0], t12 = theta[0] + theta[1];
    return {
        L1 * std::cos(t1) + L2 * std::cos(t12),
        L1 * std::sin(t1) + L2 * std::sin(t12)
    };
}

// Analytic Jacobian from your derivation
int main() {
    std::vector<double> theta = {0.5, 0.8};  // some joint config

    MatX J_num = jacobian(fk, theta);

    // Compare against analytic — print both, check they match to ~1e-7
    double t1 = theta[0], t12 = theta[0] + theta[1];

    double j00 = -std::sin(t1) - std::sin(t12);
    double j10 = std::cos(t1) + std::cos(t12);

    double j01 = -std::sin(t12);
    double j11 = std::cos(t12);


    std::cout << "Numeric:  " << J_num(0,0) << " " << J_num(0,1) << "\n";
    std::cout << "          " << J_num(1,0) << " " << J_num(1,1) << "\n";
    std::cout << "Analytic: " << j00 << " " << j01 << "\n";
    std::cout << "          " << j10 << " " << j11 << "\n";




    std::vector<double> singular = {0.5, 0.0};
    MatX J_sing = jacobian(fk, singular);

    double det_sing = J_sing(0,0) * J_sing(1,1) - J_sing(0,1) * J_sing(1,0);

    if (std::abs(det_sing) < 1e-9) {
        std::cout << "Arm extended\n";
    } 

}