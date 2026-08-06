#include "math/integrators.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>


struct State {
    double x; // position
    double v; // velocity
};

State operator+(State a, State b) { return {a.x + b.x, a.v + b.v}; }
State operator*(double s, State a) { return {s * a.x, s * a.v}; }

struct SpringMass {
    double k = 1.0, m = 1.0;

    State operator()(double t, State y) const {
        return {y.v, -(k/m) * y.x};
    }
};


int main() {
    ExplicitEuler euler;
    RK4 rk;
    SemiImplicitEuler symplectic;

    SpringMass spring{1.0, 1.0};
    auto accel = [&](double x) { return -(spring.k / spring.m) * x; };

    State y{1.0, 0.0};
    State rk_y{1.0, 0.0};
    State si_y{1.0, 0.0};

    double dt = 0.01;

    for (int i = 0; i < 1000; ++i) {
        y = euler.step(spring, i * dt, y, dt);
        rk_y = rk.step(spring, i * dt, rk_y, dt);
        si_y = symplectic.step(accel, si_y, dt);
    }

    double euler_energy = 0.5 * y.v * y.v + 0.5 * y.x * y.x;
    double rk4_energy = 0.5 * rk_y.v * rk_y.v + 0.5 * rk_y.x * rk_y.x;
    double symplectic_energy = 0.5 * si_y.v * si_y.v + 0.5 * si_y.x * si_y.x;

    std::cout << "Euler x: " << y.x << " v: " << y.v << " E: " << euler_energy << "\n";
    std::cout << "RK4   x: " << rk_y.x << " v: " << rk_y.v << " E: " << rk4_energy << "\n";
    std::cout << "SemiEuler   x: " << si_y.x << " v: " << si_y.v << " E: " << symplectic_energy << "\n";
    return 0;
}