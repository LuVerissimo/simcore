#include "math/spsc_queue.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <random>
#include <cmath>
#include <chrono>

// 4×4 matrix (fixed size for KF — no heap)
struct Mat4 {
    double m[4][4]{};
    double& operator()(int r, int c) { return m[r][c]; }
    double  operator()(int r, int c) const { return m[r][c]; }
};

struct Vec4 { double v[4]{}; double& operator[](int i) { return v[i]; } double operator[](int i) const { return v[i]; } };
struct Vec2 { double x, y; };

// Matrix ops you'll need — implement these:
Vec4 mat4_vec4(const Mat4& A, const Vec4& vec){
    return Vec4 {
        A(0,0) * vec[0] + A(0,1) * vec[1] + A(0,2) * vec[2] + A(0,3) * vec[3],
        A(1,0) * vec[0] + A(1,1) * vec[1] + A(1,2) * vec[2] + A(1,3) * vec[3],
        A(2,0) * vec[0] + A(2,1) * vec[1] + A(2,2) * vec[2] + A(2,3) * vec[3],
        A(3,0) * vec[0] + A(3,1) * vec[1] + A(3,2) * vec[2] + A(3,3) * vec[3]
    };
}

Mat4 mat4_mat4(const Mat4& A, const Mat4& B){
    Mat4 result;

    for (int i = 0; i < 4; ++i) {
        for (int k = 0; k < 4; ++k) {
            double r = A(i, k); // Cache-friendly: read once from A
            for (int j = 0; j < 4; ++j) {
                result(i, j) += r * B(k, j); // Linear sequential access on B and result
            }
        }
    }
    return result;
}

Mat4 transpose4(const Mat4& A) { 
    // Uses normal constructor arguments or a flat array constructor
    return {{
        {A(0,0), A(1,0), A(2,0), A(3,0)},
        {A(0,1), A(1,1), A(2,1), A(3,1)},
        {A(0,2), A(1,2), A(2,2), A(3,2)},
        {A(0,3), A(1,3), A(2,3), A(3,3)}
    }};
}
Mat4 add4(const Mat4& A, const Mat4& B) {
    return
       Mat4 {{
        {A(0,0) + B(0,0),  A(0,1) + B(0,1),  A(0,2) + B(0,2),  A(0,3) + B(0,3)},
        {A(1,0) + B(1,0),  A(1,1) + B(1,1),  A(1,2) + B(1,2),  A(1,3) + B(1,3)},
        {A(2,0) + B(2,0),  A(2,1) + B(2,1),  A(2,2) + B(2,2),  A(2,3) + B(2,3)},
        {A(3,0) + B(3,0),  A(3,1) + B(3,1),  A(3,2) + B(3,2),  A(3,3) + B(3,3)}
    }};
};
Mat4 sub4(const Mat4& A, const Mat4& B) {
    return
       Mat4 {{
        {A(0,0) - B(0,0),  A(0,1) - B(0,1),  A(0,2) - B(0,2),  A(0,3) - B(0,3)},
        {A(1,0) - B(1,0),  A(1,1) - B(1,1),  A(1,2) - B(1,2),  A(1,3) - B(1,3)},
        {A(2,0) - B(2,0),  A(2,1) - B(2,1),  A(2,2) - B(2,2),  A(2,3) - B(2,3)},
        {A(3,0) - B(3,0),  A(3,1) - B(3,1),  A(3,2) - B(3,2),  A(3,3) - B(3,3)}
    }};
};

// For 2×2 inverse (innovation covariance S is 2×2)
void inv2x2(double a, double b, double c, double d,
            double& ai, double& bi, double& ci, double& di) {
    double det = a*d - b*c;
    ai = d/det; bi = -b/det; ci = -c/det; di = a/det;
}

struct Measurement {
    double t;
    double px, py;
};

// --- Sensor thread: simulate noisy measurements ---
void sensor(SPSCQueue<Measurement, 1024>& q, int n_steps, double dt) {
    std::mt19937 gen(42);
    std::normal_distribution<double> noise(0, 0.5);  // R = 0.25

    // True trajectory: constant velocity
    double px = 0, py = 0, vx = 1.0, vy = 0.5;

    for (int i = 0; i < n_steps; ++i) {
        double t = i * dt;
        Measurement m{t, px + noise(gen), py + noise(gen)};
        while (!q.push(m)) {}  // spin until room

        // Advance true state
        px += vx * dt;
        py += vy * dt;

        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // simulate 100 Hz-ish
    }
}

// --- Estimator thread: run KF ---
void estimator(SPSCQueue<Measurement, 1024>& q, int n_steps, double dt) {
    // State: [px, py, vx, vy]
    Vec4 x{0, 0, 0, 0};  // initial guess

    // P = large diagonal (uncertain)
    Mat4 P{};
    P(0,0) = P(1,1) = 10; P(2,2) = P(3,3) = 1;

    // F (constant velocity)
    Mat4 F{};
    // Setup velo matrix
    F(0,0) = F(1,1) = F(2,2) = F(3,3) = 1;
    F(1,0) = F(2,1) = F(2,0) = 0;
    F(0,1) = F(2,1) = F(3,1) = 0;
    F(1,2) = F(3,2) = 0;
    F(0,3) = F(2,3) = 0;
    F(0,2) = F(1,3) = dt;

    // H (observe position)
    Mat4 H{};
    H(0,0) = H(1,1) = 1;
    H(0,1) = H(0,2) = H(0,3) = H(1,0) = H(1,2) = H(1,3) = 0;

    // Going to assumed independence for R (measurement noise) and Q (process noise)
    Mat4 Q{};
    Q(0,0) = Q(1,1) = 0.001;   // small position uncertainty
    Q(2,2) = Q(3,3) = 0.01;    // slightly more velocity uncertainty
    
    Mat4 R{};
    R(0,0) = R(1,1) = 0.25;

    std::ofstream log("kf_log.csv");
    log << "t,true_px,true_py,meas_px,meas_py,est_px,est_py\n";

    // Reconstruct true state for logging
    double true_px = 0, true_py = 0, true_vx = 1.0, true_vy = 0.5;

    for (int i = 0; i < n_steps; ++i) {
        Measurement m;
        while (!q.pop(m)) {}  // spin until data

        // state/covariance predictions
        // x = F * x
        // P = F * P * Fᵀ + Q

        // --- UPDATE ---
        // y = z - H * x         (innovation)
        // K = P * Hᵀ * S⁻¹      (4×2, but you can do it column by column) K gain'
        // auto K = (mat4_mat4(P_covar, transpose4(H)), inv2x2(S_i_covar));
        // x = x + K * y         
        // P = (I - K*H) * P

        // Log
        log << m.t << "," << true_px << "," << true_py << ","
            << m.px << "," << m.py << ","
            << x[0] << "," << x[1] << "\n";

        true_px += true_vx * dt;
        true_py += true_vy * dt;
    }
}

int main() {
    SPSCQueue<Measurement, 1024> q;
    int n_steps = 500;
    double dt = 0.01;

    std::jthread sensor_t(sensor, std::ref(q), n_steps, dt);
    std::jthread estim_t(estimator, std::ref(q), n_steps, dt);
}