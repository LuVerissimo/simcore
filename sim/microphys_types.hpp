#pragma once
#include "math/vec3.hpp"
#include "math/quat.hpp"
#include <vector>
#include <random>

const int N_BODIES = 100;
const int N_PARTICLES = 10000;

inline std::uniform_real_distribution<float> pos_dist(-20.0f, 20.0f);
inline std::uniform_real_distribution<float> height_dist(1.0f, 50.0f);

struct ParticleRef {
    float& px; float& py; float& pz; 
    float& vx; float& vy; float& vz; 

    void set_pos(const vec3f& p) { px = p.x; py = p.y; pz = p.z; }
    void set_vel(const vec3f& v) { vx = v.x; vy = v.y; vz = v.z; }

    [[nodiscard]] inline vec3f get_pos() const { return {px, py, pz};}
    [[nodiscard]] inline vec3f get_vel() const { return {vx, vy, vz};}
};

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