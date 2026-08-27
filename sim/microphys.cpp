#include "sim/microphys_types.hpp"
#include "sim/microphys_spatial.hpp"
#include "sim/microphys_collision.hpp"
#include "sim/microphys_ekf.hpp"
#include "math/thread_pool.hpp"
#include <chrono>
#include <iostream>
#include <memory_resource>
#include <random>

std::vector<char> arena_buf(1024 * 64);
std::pmr::monotonic_buffer_resource arena(arena_buf.data(), arena_buf.size());
std::mt19937 gen(42);

SPSCQueue<Observation, 2048> obs_queue;
ThreadPool pool_t(4);

int main() {
    Particles particles(N_PARTICLES, gen); {};
    Bodies bodies(gen);
    bodies.vel[0] = {3.0f, 0.0f, 1.0f};

    vec3f gravity = {0,-9.81f,0};
    float dt = 0.001f;
    float e = 0.8f; //restituion
    int steps = 20'000; //20 secs
    SpatialHash spatial_hash(2.0f);
    std::vector<std::pair<float,float>> truth_log;
    
    auto t0 = std::chrono::high_resolution_clock::now();

    for (int step = 0; step < steps; ++step) {
        std::pmr::vector<Contact> contacts(&arena);
        spatial_hash.clear();
        
        // --- Integration (parallel) ---
        auto body_future = pool_t.submit([&] {
            for (int i = 0; i < N_BODIES; ++i) {
                //linear
                bodies.vel[i] += gravity * dt;
                bodies.pos[i] += bodies.vel[i] * dt;
    
                //angular
                quat spin = {0.0f, bodies.omega[i].x, bodies.omega[i].y, bodies.omega[i].z};
                bodies.ori[i] = bodies.ori[i] + (spin * bodies.ori[i]) * (0.5f * dt);
                bodies.ori[i] = normalized(bodies.ori[i]);
    
                if (bodies.pos[i].y < bodies.radius[i]) {
                    bodies.pos[i].y = bodies.radius[i];
                    bodies.vel[i].y = -e * bodies.vel[i].y;
                    bodies.omega[i] *= 0.95f;
                }
            }
        });
      
        auto particle_future = pool_t.submit([&] {
            for (int i = 0; i < particles.count; ++i) {
                particles.vx[i] += gravity.x * dt; particles.vy[i] += gravity.y * dt; particles.vz[i] += gravity.z * dt;
                particles.px[i] += particles.vx[i] * dt; particles.py[i] += particles.vy[i] * dt; particles.pz[i] += particles.vz[i] * dt;
                
                float particle_rad = 0.05f;
                if (particles.py[i] < particle_rad) {
                    particles.py[i] = particle_rad;
                    particles.vy[i] = -e * particles.vy[i];
                }
            }
        });

        body_future.get();
        particle_future.get();

        // --- Observation (body 0, 100 Hz) ---
        if (step % 10 == 0) {
            truth_log.push_back({bodies.pos[0].x, bodies.pos[0].y});
            std::normal_distribution<float> noise(0, 0.3f);
            obs_queue.push({step * dt,
                            bodies.pos[0].x + noise(gen),
                            bodies.pos[0].y + noise(gen)});
        }

        // --- Broadphase + collision (single-threaded) ---
        for (int i = 0; i < N_BODIES; ++i) {
            spatial_hash.insert(i, bodies.pos[i].x, bodies.pos[i].y, bodies.pos[i].z);
        }
        for (int i = 0; i < particles.count; ++i) {
            spatial_hash.insert(N_BODIES + i, particles.px[i], particles.py[i], particles.pz[i]);
        }

        broadphase(bodies, particles, spatial_hash, contacts);
        resolve_contacts(contacts, bodies, particles, e);
        arena.release();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "Total: " << total_ms << " ms, per step: " << total_ms / steps << " ms\n";

    // --- EKF offline pass ---
    float obs_dt = 10 * dt;
    run_ekf_offline(obs_queue, truth_log, obs_dt);
}