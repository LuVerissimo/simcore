#pragma once
#include "matx.hpp"
#include <vector>
#include <functional>
#include <cmath>

// f: Rⁿ → Rᵐ, x is the evaluation point, h is the step size
MatX jacobian(std::function<std::vector<double>(const std::vector<double>&)> f,
              const std::vector<double>& x,
              double h = 1e-7);