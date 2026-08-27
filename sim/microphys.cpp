#include "math/quat.hpp"
#include "math/vec3.hpp"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory_resource>
#include <random>
#include <unordered_map>
#include <vector>
#include <chrono>
#include "math/thread_pool.hpp"
#include "math/spsc_queue.hpp"
#include <fstream>

const int N_BODIES = 100;
const int N_PARTICLES = 10000;

struct ParticleRef {
    float& px; float& py; float& pz; 
    float& vx; float& vy; float& vz; 

    void set_pos(const vec3f& p) { px = p.x; py = p.y; pz = p.z; }
    void set_vel(const vec3f& v) { vx = v.x; vy = v.y; vz = v.z; }

    [[nodiscard]] inline vec3f get_pos() const { return {px, py, pz};}
    [[nodiscard]] inline vec3f get_vel() const { return {vx, vy, vz};}
};

std::uniform_real_distribution<float> pos_dist(-20.0f, 20.0f);
std::uniform_real_distribution<float> height_dist(1.0f, 50.0f);

struct Particles {
    int count;
    std::vector<float> px, py, pz;
    std::vector<float> vx, vy, vz;
    
    Particles(int c, std::mt19937& gen) : count(c), px(c), py(c), pz(c), vx(c), vy(c), vz(c) {
        for (int i = 0; i < count; ++i) {
                px[i] = pos_dist(gen);
                py[i] = height_dist(gen);
                pz[i] = pos_dist(gen);
        }
    }

    inline ParticleRef operator[](int i) {
        return ParticleRef {px[i], py[i], pz[i], vx[i], vy[i], vz[i] };
    }
    [[nodiscard]] inline vec3f get_pos(int i) const { return {px[i], py[i], pz[i]};}
    [[nodiscard]] inline vec3f get_vel(int i) const { return {vx[i], vy[i], vz[i]};}
};

struct Bodies {
    vec3f pos[N_BODIES];
    vec3f vel[N_BODIES];
    vec3f omega[N_BODIES];
    quat ori[N_BODIES];
    float mass[N_BODIES];
    float radius[N_BODIES];
    float inv_inertia[N_BODIES];

    Bodies(std::mt19937& gen) {
        std::uniform_real_distribution<float> rad_dist(0.3f, 1.0f);
        for (int i = 0; i < N_BODIES; ++i) {
            pos[i] = {pos_dist(gen), height_dist(gen), pos_dist(gen)};
            vel[i] = {0, 0, 0};
            omega[i] = {0, 0, 0};
            ori[i] = {1, 0, 0, 0};
            mass[i] = 1.0f;
            radius[i] = rad_dist(gen);

            float inertia = 0.4f * mass[i] * radius[i] * radius[i];
            inv_inertia[i] = 1.0f / inertia;
        }
    }
};

struct Contact { 
    int id_a, id_b;
    bool is_body_a, is_body_b; 
    vec3f normal; 
    float overlap; 
};

std::vector<char> arena_buf(1024 * 64);
std::pmr::monotonic_buffer_resource pool(arena_buf.data(), arena_buf.size());
std::mt19937 gen(42);

struct SpatialHash {
    float cell_size;
    std::unordered_map<uint64_t, std::vector<int>> grid;

    SpatialHash(float cs) : cell_size(cs) {}

    uint64_t hash(float x, float y, float z) const {
        int cx = (int)std::floor(x / cell_size);
        int cy = (int)std::floor(y / cell_size);
        int cz = (int)std::floor(z / cell_size);
        
        // Pack three ints into one uint64_t using prime multipliers to reduce collisions
        return (uint64_t)((cx * 73856093) ^ (cy * 19349663) ^ (cz * 83492791));
    }
    
    void clear() { grid.clear(); }

    void insert(int global_id, float x, float y, float z) {
        grid[hash(x,y,z)].push_back(global_id);
    }
};

ThreadPool pool_t(4);

struct Observation { float t, px, py; };
SPSCQueue<Observation, 2048> obs_queue;

int main() {
    Particles particles(N_PARTICLES, gen); {};
    Bodies bodies(gen);
    bodies.vel[0] = {3.0f, 0.0f, 1.0f};

    vec3f gravity = {0,-9.81f,0};
    float dt = 0.001f;
    float e = 0.8f; //restituion
    int steps = 20'000; //20 secs
    SpatialHash spatial_hash(2.0f);
    
    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<std::pair<float,float>> truth_log;

    for (int step = 0; step < steps; ++step) {
        std::pmr::vector<Contact> contacts(&pool);
        spatial_hash.clear();
        
        // update bodies
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
      
        // update particles
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


        if (step % 10 == 0) {
            truth_log.push_back({bodies.pos[0].x, bodies.pos[0].y});
            std::normal_distribution<float> noise(0, 0.3f);
            obs_queue.push({step * dt,
                            bodies.pos[0].x + noise(gen),
                            bodies.pos[0].y + noise(gen)});
        }

        // Build spatial hash after integration to avoid race condition
        for (int i = 0; i < N_BODIES; ++i) {
            spatial_hash.insert(i, bodies.pos[i].x, bodies.pos[i].y, bodies.pos[i].z);
        }

        // broadlook up optimization(group objects by proximity)
        for (auto& [cell_hash, body_ids] : spatial_hash.grid) {
            int bodies_in_cell = (int)body_ids.size(); 
            for (int a = 0; a < bodies_in_cell; ++a) {
                for (int b = a + 1; b < bodies_in_cell; ++b) {
                    int id_a = body_ids[a]; int id_b = body_ids[b];
                    bool is_body_a = (id_a < N_BODIES); bool is_body_b = (id_b < N_BODIES);

                    vec3f pos_a = is_body_a ? bodies.pos[id_a] : particles.get_pos(id_a - N_BODIES);
                    vec3f pos_b = is_body_b ? bodies.pos[id_b] : particles.get_pos(id_b - N_BODIES);

                    float rad_a = is_body_a ? bodies.radius[id_a] : 0.05f;
                    float rad_b = is_body_b ? bodies.radius[id_b] : 0.05f;

                    vec3f diff = pos_a - pos_b;
                    float dist = norm(diff);
                    float combined_radius = rad_a + rad_b;

                    if (dist < combined_radius && dist > 0.0001f) {
                        contacts.push_back({id_a, id_b, is_body_a, is_body_b, diff/dist, combined_radius - dist});
                    }
                }
            }
        }

        // Same cell impulse loop
        for (auto& c : contacts) {
            int idx_a = c.is_body_a ? c.id_a : (c.id_a - N_BODIES);
            int idx_b = c.is_body_b ? c.id_b : (c.id_b - N_BODIES);

            vec3f pos_a = c.is_body_a ? bodies.pos[idx_a] : particles.get_pos(idx_a);
            vec3f pos_b = c.is_body_b ? bodies.pos[idx_b] : particles.get_pos(idx_b);

            // contact point is the midpoint of overlapping boundaries
            float rad_a = c.is_body_a ? bodies.radius[idx_a] : 0.05f;
            vec3f contact_point = pos_a - c.normal * (rad_a - (c.overlap * 0.5f));

            // levarage arms
            vec3f r_a = contact_point - pos_a;
            vec3f r_b = contact_point - pos_b;
            
            //total vel at contact point = linear + angular vel
            vec3f vel_a = c.is_body_a ? (bodies.vel[idx_a] + cross(bodies.omega[idx_a], r_a)) : particles.get_vel(idx_a);
            vec3f vel_b = c.is_body_b ? (bodies.vel[idx_b] + cross(bodies.omega[idx_b], r_b)) : particles.get_vel(idx_b);


            float inv_mass_a = c.is_body_a ? (1.0f / bodies.mass[idx_a]) : (1.0f / 0.1f);
            float inv_mass_b = c.is_body_b ? (1.0f / bodies.mass[idx_b]) : (1.0f / 0.1f);

            float v_rel = dot(vel_a - vel_b, c.normal);
            if (v_rel < 0.0f) {
                // Angular Rot Inertia 
                float angular_inertia_a = c.is_body_a ? dot(cross(r_a, c.normal) * bodies.inv_inertia[idx_a], cross(r_a, c.normal)) : 0.0f;
                float angular_inertia_b = c.is_body_b ? dot(cross(r_b, c.normal) * bodies.inv_inertia[idx_b], cross(r_b, c.normal)) : 0.0f;

                // Full denominator with linear and angular mass
                float denominator = inv_mass_a + inv_mass_b + angular_inertia_a + angular_inertia_b;

                float j = -(1.0f + e) * v_rel / denominator;
                vec3f impulse = c.normal * j;
                
                // Mutate properties for A
                if (c.is_body_a) {
                    bodies.vel[idx_a] += impulse * inv_mass_a;
                    bodies.omega[idx_a] += cross(r_a, impulse) * bodies.inv_inertia[idx_a];
                    bodies.pos[idx_a] += c.normal * (c.overlap * 0.5f);
                } else {
                    auto p_a = particles[idx_a];
                    p_a.vx += impulse.x * inv_mass_a; p_a.vy += impulse.y * inv_mass_a; p_a.vz += impulse.z * inv_mass_a;
                    p_a.px += c.normal.x * (c.overlap * 0.5f); p_a.py += c.normal.y * (c.overlap * 0.5f); p_a.pz += c.normal.z * (c.overlap * 0.5f); 
                }
                
                // Mutate properties for B
                if (c.is_body_b) {
                    bodies.vel[idx_b] -= impulse * inv_mass_b;
                    bodies.omega[idx_b] -= cross(r_b, impulse) * bodies.inv_inertia[idx_b];
                    bodies.pos[idx_b] -= c.normal * (c.overlap * 0.5f);
                } else {
                    auto p_b = particles[idx_b];
                    p_b.vx -= impulse.x * inv_mass_b; p_b.vy -= impulse.y * inv_mass_b; p_b.vz -= impulse.z * inv_mass_b;
                    p_b.px -= c.normal.x * (c.overlap * 0.5f); p_b.py -= c.normal.y * (c.overlap * 0.5f); p_b.pz -= c.normal.z * (c.overlap * 0.5f); 
                }
            }
        }
        

    }

    auto t1 = std::chrono::high_resolution_clock::now();


    // --- EKF offline pass ---
    std::ofstream log("microphys_ekf.csv");
    log << "t,true_px,true_py,meas_px,meas_py,est_px,est_py\n";

    // KF state: [px, py, vx, vy]
    double xk[4] = {0, 0, 0, 0};
    double P[4][4] = {{10,0,0,0},{0,10,0,0},{0,0,1,0},{0,0,0,1}};
    double Q[4][4] = {{0.001,0,0,0},{0,0.001,0,0},{0,0,0.01,0},{0,0,0,0.01}};
    double R_kf[2][2] = {{0.09, 0}, {0, 0.09}};  // sigma=0.3 → var=0.09
    double obs_dt = 10 * dt;  // 100 Hz

    Observation obs;
    int obs_inx = 0;
    while (obs_queue.pop(obs)) {
        auto [tx, ty] = truth_log[obs_inx++];

        // PREDICT
        xk[0] += xk[2] * obs_dt;
        xk[1] += xk[3] * obs_dt;
        // P = F P Fᵀ + Q (F is identity + dt in [0][2],[1][3])
        P[0][0] += 2*obs_dt*P[0][2] + obs_dt*obs_dt*P[2][2] + Q[0][0];
        P[1][1] += 2*obs_dt*P[1][3] + obs_dt*obs_dt*P[3][3] + Q[1][1];
        P[0][2] += obs_dt*P[2][2]; P[2][0] = P[0][2];
        P[1][3] += obs_dt*P[3][3]; P[3][1] = P[1][3];
        P[2][2] += Q[2][2]; P[3][3] += Q[3][3];

        // UPDATE
        // S = H P Hᵀ + R (H = [I₂ 0])
        double S[2][2] = {{P[0][0]+R_kf[0][0], P[0][1]},
                        {P[1][0], P[1][1]+R_kf[1][1]}};
        double det = S[0][0]*S[1][1] - S[0][1]*S[1][0];
        double Si[2][2] = {{S[1][1]/det, -S[0][1]/det},
                            {-S[1][0]/det, S[0][0]/det}};

        // K = P Hᵀ S⁻¹ (4×2)
        double K[4][2];
        for (int i = 0; i < 4; ++i) {
            K[i][0] = P[i][0]*Si[0][0] + P[i][1]*Si[1][0];
            K[i][1] = P[i][0]*Si[0][1] + P[i][1]*Si[1][1];
        }

        // innovation
        double y[2] = {obs.px - xk[0], obs.py - xk[1]};
        for (int i = 0; i < 4; ++i)
            xk[i] += K[i][0]*y[0] + K[i][1]*y[1];

        // P = (I - KH) P
        double KH[4][4] = {};
        for (int i = 0; i < 4; ++i) { KH[i][0] = K[i][0]; KH[i][1] = K[i][1]; }
        double P_new[4][4] = {};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    double ikh = (i==k ? 1.0 : 0.0) - KH[i][k];
                    P_new[i][j] += ikh * P[k][j];
                }
            }
        }
        // Simplified: just copy P update
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                P[i][j] = P_new[i][j];

        log << obs.t << "," << tx << "," << ty << ","
            << obs.px << "," << obs.py << ","
            << xk[0] << "," << xk[1] << "\n";
    }
    std::cout << "EKF log written to microphys_ekf.csv\n";

    double total_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << "Total: " << total_ms << " ms, per step: " << total_ms / steps << " ms\n";
}