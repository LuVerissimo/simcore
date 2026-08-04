#pragma once
#include "matx.hpp"
#include <vector>
#include <functional>
#include <cmath>

// f: Rⁿ → Rᵐ, x is the evaluation point, h is the step size
MatX jacobian(std::function<std::vector<double>(const std::vector<double>&)> f,
              const std::vector<double>& x,
              double h = 1e-7)
{
    auto f_at_x =  f(x);
    int m = f_at_x.size();
    int n = x.size();
    MatX J(m, n);

    for (int j = 0; j < n; ++j) {
        auto x_plus = x;
        auto x_minus = x;
        x_plus[j] += h;
        x_minus[j] -= h;


        //out vectors size M;
        auto f_plus = f(x_plus);
        auto f_minus = f(x_minus);

        for (int i =  0; i < m; ++i) {
            J(i,j) = (f_plus[i] - f_minus[i]) / (2.0 * h);
        }
    }
    return J;
}