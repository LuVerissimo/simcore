#include <iostream>
#include <string>
#include <vector>

struct Tracer {
    std::string name;

    Tracer(std::string n) : name(std::move(n)) {
        std::cout << "[CTOR] " << name << "\n";
    }

    Tracer(const Tracer& other) : name(other.name + " (copy)") {
        std::cout << "[COPY] " << name << "\n";
    }

    Tracer& operator=(const Tracer& other) {
        name = other.name;
        std::cout << "[COPY=] " << name << "\n";
        return *this;
    }

    ~Tracer() {
        std::cout << "[DTOR] " << name << "\n";
    }
};

// Exp 2
// void by_value(Tracer t) {
//     std::cout << "  inside by_value\n";
// }
// void by_ref(const Tracer& t) {
//     std::cout << "  inside by_ref\n";
// }

// Exp3
// Tracer make() {
//     Tracer t("A");
//     return t;
// }

// int main() {
    // exp2
    // Tracer a("A");
    // std::cout << "--- by value ---\n";
    // by_value(a);
    // std::cout << "--- by ref ---\n";
    // by_ref(a);

    // Exp3 
    // Tracer a = make();
// }

int main() {
    std::cout << "--- without reserve ---\n";
    std::vector<Tracer> v;
    v.reserve(3);
    v.push_back(Tracer("A"));
    v.push_back(Tracer("B"));
    v.push_back(Tracer("C"));
    std::cout << "--- done ---\n";
}