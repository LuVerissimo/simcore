#include "math/vec3.hpp"
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

const int N_BODIES = 100;

struct ParticleRef {
    float& px; float& py; float& pz; 
    float& vx; float& vy; float& vz; 

    void set_pos(const vec3f& p) { px = p.x; py = p.y; pz = p.z; }
    void set_vel(const vec3f& v) { vx = v.x; vy = v.y; vz = v.z; }
};

struct Particles {
    // SoA position
    int count;
    float px[N_BODIES]; float py[N_BODIES]; float pz[N_BODIES]; 

    // SoA velocity
    float vx[N_BODIES]; float vy[N_BODIES]; float vz[N_BODIES];

    inline ParticleRef operator[](int i) {
        return ParticleRef {px[i], py[i], pz[i], vx[i], vy[i], vz[i] };
    }
};

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

    void insert(int body_id, float x, float y, float z) {
        grid[hash(x,y,z)].push_back(body_id);
    }
};

int main() {
    Particles particles(10000); {};
    particles.pos = {px, py, pz};
    vec3f gravity = {0,-9.81f,0};
    float dt = 0.001f;
    double e = 0.8; //restituion
    int steps = 20'000; //20 secs

    SpatialHash spatial_hash(2);
 
    for (int step = 0; step < steps; ++step) {
        spatial_hash.clear();

        for (int i = 0; i < N_BODIES; ++i) {
            auto p  = particles[i];
            spatial_hash.insert(i, p.px, p.py,  p.pz);

            p.vx += gravity.x * dt;
            p.vy += gravity.y * dt;
            p.vz += gravity.z * dt;
            
            p.px += p.vx * dt;
            p.px += p.vy * dt;
            p.px += p.vz * dt;
        }

        // collision loop that only tests pairs in the same cell
        for (auto& [cell_hash, body_ids] : spatial_hash.grid) {
            for (int a = 0; a < (int)body_ids.size(); ++a) {
                for (int b = a + 1; b < (int)body_ids.size(); ++b) {
                    vec3f diff = {particles.px[a] - particles.px[b],particles.py[a] - particles.py[b],particles.pz[a] - particles.pz[b] };
                    float dist = norm(diff);

                }
            }
        }
    }

}