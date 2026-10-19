#pragma once
#include <vector>
#include "simcore/cuda/device_buffer.hpp"

namespace simcore::cuda {
    [[nodiscard]] double naive_reduction_dot(const std::vector<double>& a, const std::vector<double>& b);
    [[nodiscard]] double naive_reduction_dot(const double* d_a, const double* d_b, int n);


    [[nodiscard]] double sequential_addressing_reduction_dot(const std::vector<double>& a, const std::vector<double>& b);
    [[nodiscard]] double warp_shuffle_reduction_dot(const std::vector<double>& a, const std::vector<double>& b);
}