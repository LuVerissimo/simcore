const int N_BODIES = 2;

struct vec3f {
    float x, y, z;
};
struct ParticleRef {
    float& px; float& py; float& pz; 
    float& vx; float& vy; float& vz; 

    void set_pos(const vec3f& p) { px = p.x; py = p.y; pz = p.z; }
    void set_vel(const vec3f& v) { vx = v.x; vy = v.y; vz = v.z; }
};

struct Particles {
    // SoA position
    float px[N_BODIES]; float py[N_BODIES]; float pz[N_BODIES]; 

    // SoA velocity
    float vx[N_BODIES]; float vy[N_BODIES]; float vz[N_BODIES];

    inline ParticleRef operator[](int i) {
        return ParticleRef {px[i], py[i], pz[i], vx[i], vy[i], vz[i] };
    }
};



int main() {
    Particles particles{};
    vec3f gravity = {0,-9.81f,0};
    float dt = 0.001f;
    float e = 0.8f; //restituion
    int steps = 20'000; //20 secs

    for (int step = 0; step < steps; ++step) {
        for (int i = 0; i < N_BODIES; ++i) {
            auto p  = particles[i];

            p.vx += gravity.x * dt;
            p.vy += gravity.y * dt;
            p.vz += gravity.z * dt;
            
            p.px += p.vx * dt;
            p.px += p.vy * dt;
            p.px += p.vz * dt;
        }

        // collision loop
        for (int a = 0; a < N_BODIES; ++a) {
            for (int b = 0; b < N_BODIES; ++b) {
            }
        }
    }

}