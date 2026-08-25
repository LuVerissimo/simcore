#include "math/vec3.hpp"
#include "math/quat.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory_resource>
#include <vector>

const int N_BODIES = 2;

struct Bodies {
    vec3 pos[N_BODIES];
    vec3 vel[N_BODIES];
    vec3 omega[N_BODIES];
    quat ori[N_BODIES];
    double mass[N_BODIES];
    double radius[N_BODIES];
};
struct Contact { int a, b; vec3 normal; double overlap; };
std::vector<char> arena_buf(1024 * 64);
std::pmr::monotonic_buffer_resource pool(arena_buf.data(), arena_buf.size());

int main() {
    Bodies bodies;

    bodies.pos[0] = {0,0.5,0};
    bodies.vel[0] = {2,0,0};
    bodies.omega[0] = {1.0,2.0,0.5};
    bodies.ori[0] = {1,0,0,0};
    bodies.mass[0] = 1.0;
    bodies.radius[0] = 0.5;
    
    bodies.pos[1] = {30,0.5,0}; //sitting on the ground, b's path
    bodies.vel[1] = {0,0,0};
    bodies.omega[1] = {0,0,0};
    bodies.ori[1] = {1,0,0,0};
    bodies.mass[1] = 1.0;
    bodies.radius[1] = 0.5;
    

    vec3 gravity = {0,-9.81,0};
    double dt = 0.001;
    double e = 0.8; //restituion
    int steps = 20'000; //20 secs

    auto total_energy = [&]() {
        double E = 0;
        for (int b = 0; b < N_BODIES; ++b) {
            E += 0.5 * bodies.mass[b] * norm_squared(bodies.vel[b]);
            E += bodies.mass[b] * bodies.pos[b].y * gravity.y;
        }
        return E;
    };

    double E0 = total_energy();
    for (int i = 0; i < steps; ++i) {
        std::pmr::vector<Contact> contacts(&pool);

        for (int b = 0; b < N_BODIES; ++b) {
            //semi-implicit euler
            bodies.vel[b] += gravity * dt;
            quat wq{0, bodies.omega[b].x, bodies.omega[b].y, bodies.omega[b].z};

            bodies.ori[b] = normalized(bodies.ori[b] + (wq * bodies.ori[b]) * 0.5 * dt);
            bodies.pos[b] += bodies.vel[b] * dt;
            
            if (bodies.pos[b].y < bodies.radius[b]) {
                bodies.pos[b].y = bodies.radius[b];
                bodies.vel[b].y = -e * bodies.vel[b].y;
            }
        }

        //collision detection fills contacts
        for (int a = 0; a < N_BODIES; ++a) {
            for (int b = a + 1; b < N_BODIES; ++b) {
                vec3 diff = bodies.pos[a] - bodies.pos[b];
                double dist = norm(diff);
                
                if (dist < bodies.radius[a] + bodies.radius[b]) {
                    contacts.push_back({a,b, diff/dist, bodies.radius[a] + bodies.radius[b] - dist});
                }
                
            }
        }
        for (auto& c : contacts) {
            double v_rel = dot(bodies.vel[c.a] - bodies.vel[c.b], c.normal);
                if (v_rel < 0) {
                    double j = -(1 + e) * v_rel / (1/bodies.mass[c.a] + 1/bodies.mass[c.b]);
                    bodies.vel[c.a] += c.normal * (j / bodies.mass[c.a]);
                    bodies.vel[c.b] -= c.normal * (j / bodies.mass[c.b]);
                    bodies.pos[c.a] += c.normal * (c.overlap * 0.5);
                    bodies.pos[c.b] -= c.normal * (c.overlap * 0.5);
                }
        }
        pool.release();

        if (i % 1000 == 0) std::cout << "t=" << i*dt << 
                            " b1.x=" << bodies.pos[0].x << " b1.y=" << bodies.pos[0].y <<
                            " b2.x=" << bodies.pos[1].x << " b2.y=" << bodies.pos[1].y <<
                            " E=" << total_energy() << " E0=" << E0 <<
                            " q_norm=" << norm(bodies.ori[0]) << "\n";
    }
}