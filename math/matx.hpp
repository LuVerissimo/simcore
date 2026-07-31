#pragma once
#include <memory>
#include <cassert>

class MatX {
    std::unique_ptr<double[]> data_;
    int rows_, cols_;
public:
    MatX(int rows, int cols)
        : data_(std::make_unique<double[]>(rows * cols)), rows_(rows), cols_(cols) {}

    double  operator()(int r, int c) const { return data_[r * cols_ + c]; }
    double& operator()(int r, int c)       { return data_[r * cols_ + c]; }

    int rows() const { return rows_; }
    int cols() const { return cols_; }
};