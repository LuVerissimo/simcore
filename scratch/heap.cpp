#include <memory>

class HeapArray {
    std::unique_ptr<double[]> data_;
    int size_;
public:
    HeapArray(int n) : data_(std::make_unique<double[]>(n)), size_(n) {}
    double& operator[](int i) { return data_[i]; }
    int size() const { return size_; }
};