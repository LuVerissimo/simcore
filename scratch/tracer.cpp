#include <iostream>
#include <string>

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

// void by_value(Tracer t) {
//     std::cout << "  inside by_value\n";
// }

// void by_ref(const Tracer& t) {
//     std::cout << "  inside by_ref\n";
// }


Tracer make() {
    Tracer t("A");
    return t;
}

int main() {
    // Tracer a("A");
    // std::cout << "--- by value ---\n";
    // by_value(a);
    // std::cout << "--- by ref ---\n";
    // by_ref(a);

    Tracer a = make();
}