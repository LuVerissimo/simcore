#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <cassert>
#include <cmath>
#include <utility>
#include <iostream>
#include "simcore/cuda/reduction.cuh"
#include <vector>

TEST_CASE("GPU Dot product matches CPU CG") {
    using simcore::cuda::naive_reduction_dot;
    
    const std::vector<double> c{2.0 , -1.0 , 3.0};
    const std::vector<double> d{1.0 ,4.0 ,-2.0 };

    auto x_cpu = 0.0;
    for (size_t  i = 0; i < c.size(); ++i) {
        x_cpu += c[i] * d[i];
    }
    auto x_gpu = naive_reduction_dot(c, d); // -8.0

    CHECK(x_gpu == doctest::Approx(x_cpu)); 
}