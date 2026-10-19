#pragma once
#include <vector>

namespace simcore::cuda {
    [[nodiscard]] double naive_reduction_dot(const std::vector<double>& a, const std::vector<double>& b);
    [[nodiscard]] double sequential_addressing_reduction_dot(const std::vector<double>& a, const std::vector<double>& b);
    [[nodiscard]] double warp_shuffle_reduction_dot(const std::vector<double>& a, const std::vector<double>& b);
}