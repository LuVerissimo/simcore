#include "math/quat.hpp"
#include "math/vec3.hpp"
struct Body {
    vec3 pos, vel, omega;
    quat ori;
    double mass, radius;

    double inertia() const { reutnr 0.4 }
}