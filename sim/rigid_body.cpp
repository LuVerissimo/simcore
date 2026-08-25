#include "math/sparse.hpp"
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
    b.pos = {0,0.5,0};
    b.vel = {2,0,0};
    b.omega = {1.0,2.0,0.5};
    b.ori = {1,0,0,0};
    b.mass = 1.0;
    b.radius = 0.5;
    
    Body b2;
    b2.pos = {30,0.5,0}; //sitting on the ground, b's path
    b2.vel = {0,0,0};
    b2.omega = {0,0,0};
    b2.ori = {1,0,0,0};
    b2.mass = 1.0;
    b2.radius = 0.5;
    

    vec3 gravity = {0,-9.81,0};
    double dt = 0.001;
    double e = 0.8; //restituion
    int steps = 20'000; //20 secs


    auto kinetic = [](const Body& b) { return 0.5 * b.mass * norm_squared(b.vel); };
    auto potential = [&](const Body& b) {return b.mass * b.pos.y * gravity.y; };
    double E0 = kinetic(b) + potential(b) + kinetic(b2) + potential(b2);
    
    for (int i = 0; i < steps; ++i) {
        //semi-implicit euler
        b.vel += gravity * dt;
        quat omega_q{0, b.omega.x, b.omega.y, b.omega.z};

        b.ori = b.ori + (omega_q * b.ori) * 0.5 * dt;
        b.ori = normalized(b.ori);
        
        b.pos += b.vel * dt;
        
        if (b.pos.y < b.radius) {
            b.pos.y = b.radius;
            b.vel.y = -e * b.vel.y;
        }

        b2.vel += gravity * dt;
        b2.pos += b2.vel * dt;
        if (b2.pos.y < b2.radius) {
            b2.pos.y = b2.radius;
            b2.vel.y = -e * b2.vel.y;
        }

        double E = kinetic(b) + potential(b) + kinetic(b2) + potential(b2);

        if (i % 1000 == 0) std::cout << "t=" << i*dt << 
                               " b1.x=" << b.pos.x << " b1.y=" << b.pos.y <<
                               " b2.x=" << b2.pos.x << " b2.y=" << b2.pos.y <<
                               " E=" << E << " E0=" << E0 <<
                               " q_norm=" << norm(b.ori) << "\n";
                               
        vec3 diff = b.pos - b2.pos;
        double dist = norm(diff);

        if (dist < b.radius + b2.radius) {
            vec3 n = diff/dist;
            
            double v_rel = dot(b.vel - b2.vel, n);
            if (v_rel < 0) {
                double j = -(1 + e) * v_rel / (1/b.mass + 1/b2.mass);
                b.vel += n * (j / b.mass);
                b2.vel -= n * (j / b2.mass);

                // seperate bodies
                double overlap = b.radius + b2.radius - dist;
                b.pos += n * overlap * 0.5;
                b2.pos -= n * overlap * 0.5;
            }
        }
    }
}