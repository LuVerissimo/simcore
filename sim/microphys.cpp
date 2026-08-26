#include "math/quat.hpp"
#include "math/vec3.hpp"
#include <cmath>
#include <cstdint>
#include <memory_resource>
#include <random>
#include <unordered_map>
#include <vector>

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
        int cx = (int)std::floorf(x / cell_size);
        int cy = (int)std::floorf(y / cell_size);
        int cz = (int)std::floorf(z / cell_size);
        
        // Pack three ints into one uint64_t using prime multipliers to reduce collisions
        return (uint64_t)((cx * 73856093) ^ (cy * 19349663) ^ (cz * 83492791));
    }
    
    void clear() { grid.clear(); }

    void insert(int global_id, float x, float y, float z) {
        grid[hash(x,y,z)].push_back(global_id);
    }
};

int main() {
    Particles particles(N_PARTICLES, gen); {};
    Bodies bodies(gen);

    vec3f gravity = {0,-9.81f,0};
    float dt = 0.001f;
    float e = 0.8f; //restituion
    int steps = 20'000; //20 secs
    SpatialHash spatial_hash(2.0f);
 
    for (int step = 0; step < steps; ++step) {
        std::pmr::vector<Contact> contacts(&pool);
        spatial_hash.clear();
        

        // update bodies
        for (int i = 0; i < N_BODIES; ++i) {
            spatial_hash.insert(i, bodies.pos[i].x, bodies.pos[i].y, bodies.pos[i].z);

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

        // update particles
        for (int i = 0; i < particles.count; ++i) {
            int global_id = N_BODIES + 1;
            spatial_hash.insert(global_id, particles.px[i], particles.py[i], particles.pz[i]);

            particles.vx[i] += gravity.x * dt; particles.vy[i] += gravity.y * dt; particles.vz[i] += gravity.z * dt;
            particles.vx[i] += particles.vx[i] * dt; particles.vy[i] += particles.vy[i] * dt; particles.vz[i] += particles.vz[i] * dt;

            float particle_rad = 0.05f;
            if (particles.py[i] < particle_rad) {
                particles.py[i] = particle_rad;
                particles.vy[i] = -e * particles.vy[i];
            }
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
                        contacts.push_back({id_a, id_b, is_body_a, is_body_b diff/dist, combined_radius - dist});
                    }
                }
            }
        }

        // Same cell impulse loop
    }

}