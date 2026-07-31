#include <iostream>
#include <vector>

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
    
    // move CTO
    HeapArray(HeapArray&& other) noexcept : data_(nullptr), size_(0) {

        data_ = other.data_;
        size_ = other.size_;

        other.data_ = nullptr;
        other.size_ = 0;
    }
    HeapArray& operator=(HeapArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;

            data_ = other.data_;
            size_ = other.size_;

            other.data_ = nullptr;
            other.size_ = 0;
        }

        return *this;
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

};

// int main() {
//     // HeapArray arr(5);

//     // arr[0] = 3.14;
//     // std::cout << "Element 0: " << arr[0] << std::endl;
//     // std::cout << "Array size: " << arr.size() << std::endl;

//     // return 0;

//     // Double Free
//     // HeapArray a(5);
//     // a[0] = 3.14;
//     // HeapArray b = a;   // shallow copy — b.data_ == a.data_
//     // b[0] = 999;
//     // std::cout << a[0] << "\n";  // 999 — your original got mutated

//     //use-after-free
//     double* stolen;
//     {
//         HeapArray a(5);
//         a[0] = 3.14;
//         stolen = &a[0];
//     }   // a destroyed here
//     std::cout << *stolen << "\n";   // dangling pointer
// }