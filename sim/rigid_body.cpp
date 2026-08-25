#include "math/vec3.hpp"
#include "math/quat.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

struct Body {
    vec3 pos, vel, omega;
    quat ori;
    double mass, radius;

    // sphere, I = 2/5 mr*r
    double inertia() const { return 0.4 * mass * radius * radius; }
};


int main() {
    Body b;
    b.pos = {0,0,10};
    b.vel = {2,0,0};
    b.omega = {0,0,0};
    b.ori = {1,0,0,0};
    b.mass = 1.0;
    b.radius = 0.5;


    vec3 gravity = {0,0,-9.81};
    double dt = 0.001;
    double e = 0.8; //restituion
    int steps = 20'000; //20 secs

    double normed_orientation;
    vec3 p1 = b.pos;
    // vec3 p2; //
    // vec3 delta_p = p1-p2;
    // double d = sqrt(delta_p.x * delta_p.x + delta_p.y * delta_p.y + delta_p.z * delta_p.z);
    
    
    for (int i = 0; i < steps; ++i) {
        //semi-implicit euler
        b.vel += gravity * dt;
        quat omega_q{0, b.omega.x, b.omega.y, b.omega.z};\

        b.ori = b.ori + (omega_q * b.ori) * 0.5 * dt;
        b.pos += b.vel * dt;
        normed_orientation = norm(b.ori);
        
        if (b.pos.z < b.radius) {
            b.pos.z = b.radius;
            b.vel.z = -e * b.vel.z;
        }

        if (i % 1000 == 0) std::cout << "t=" << i*dt << " z=" << b.pos.z << " norm_q=" << normed_orientation << "\n";
    }
}