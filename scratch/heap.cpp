#include <iostream>

class HeapArray {
    double* data_;
    int size_;
public:

    // CTOR
    HeapArray(int n) { // allocate with new[]
        size_ = n;
        data_ = new double[size_]();
    }         

    //DTOR     
    ~HeapArray() {  // delete[]
        delete[] data_;
    }                  

    // deep copy CTOR 
    HeapArray(const HeapArray& other){ 
        size_ = other.size_;
        data_ = new double[size_];
        for (int i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }   

    HeapArray& operator=(const HeapArray& other) {
        if (this == &other) {
            return *this;
        }

        delete[] this->data_;

        this->size_ = other.size_;

        this->data_ = new double[this->size_];

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
    HeapArray arr(5);

    arr[0] = 3.14;
    std::cout << "Element 0: " << arr[0] << std::endl;
    std::cout << "Array size: " << arr.size() << std::endl;

    return 0;
}