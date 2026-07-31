#include <iostream>
#include <vector>

class HeapArray {
    double* data_;
    int size_;
public:

    // CTOR
    HeapArray(int n) : data_(new double[n]()), size_(n) {
        std::cout << "CTOR " << size_ << "\n";
    }         

    //DTOR     
    ~HeapArray() { 
        std::cout << "DTOR " << size_ << "\n";
        delete[] data_;
    }                  

    // deep copy CTOR 
    HeapArray(const HeapArray& other) : data_(new double[other.size_]()), size_(other.size_) { 
        std::cout << "COPY CTOR " << size_ << "\n";
        for (int i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    
    // move CTOR
    HeapArray(HeapArray&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "MOVE CTOR" << size_ << "\n";
    }

    //move assignment
    HeapArray& operator=(HeapArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;

            data_ = other.data_;
            size_ = other.size_;

            other.data_ = nullptr;
            other.size_ = 0;
            std::cout << "MOVE Assignment " << size_ << "\n";
        }

        return *this;
    }

    //
    HeapArray& operator=(const HeapArray& other) noexcept {
        if (this == &other) {
            return *this;
        }

        delete[] this->data_;

        this->size_ = other.size_;

        this->data_ = new double[this->size_];
        std::cout << "COPY Assignment " << size_ << "\n";

        for (int i = 0; i < this->size_; ++i) {
            this->data_[i] = other.data_[i];
        }

        return *this;
    }

    double& operator[](int i) {
        return data_[i];
    }

    int size() const {
        return size_;
    }
};

int main() {
    std::vector<HeapArray> v;
    v.reserve(3);
    v.push_back(HeapArray(100));
    v.push_back(HeapArray(200));
    v.push_back(HeapArray(300));
};