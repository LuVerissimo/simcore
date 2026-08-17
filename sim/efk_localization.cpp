#include <cmath>
#include <utility>
#include "math/thread_pool.hpp"

// Unicycle Dynamics
struct State { double x, y, theta; };
struct Control {double v, omega; };
struct Landmark { double lx, ly; };

// Measurement model
struct Meas { double range, bearing; };

// EKF predict + update 
struct Mat3 {
    double m[3][3]{};
    double& operator()(int r, int c) { return m[r][c]; }
    double  operator()(int r, int c) const { return m[r][c]; }
};

Mat3 mat3_mat3(const Mat3& A, const Mat3& B){
    Mat3 result;

    for (int i = 0; i < 3; ++i) {
        for (int k = 0; k < 3; ++k) {
            double r = A(i, k);
            for (int j = 0; j < 3; ++j) {
                result(i, j) += r * B(k, j);
            }
        }
    }
    return result;
};

Mat3 transpose3(const Mat3& A) { 
    return {{
        {A(0,0), A(1,0), A(2,0),},
        {A(0,1), A(1,1), A(2,1),},
        {A(0,2), A(1,2), A(2,2),}
    }};
};
Mat3 add3(const Mat3& A, const Mat3& B) {
    return
       Mat3 {{
        {A(0,0) + B(0,0),  A(0,1) + B(0,1),  A(0,2) + B(0,2),},
        {A(1,0) + B(1,0),  A(1,1) + B(1,1),  A(1,2) + B(1,2),},
        {A(2,0) + B(2,0),  A(2,1) + B(2,1),  A(2,2) + B(2,2),}
    }};
};
Mat3 sub3(const Mat3& A, const Mat3& B) {
    return
       Mat3 {{
        {A(0,0) - B(0,0),  A(0,1) - B(0,1),  A(0,2) - B(0,2),},
        {A(1,0) - B(1,0),  A(1,1) - B(1,1),  A(1,2) - B(1,2),},
        {A(2,0) - B(2,0),  A(2,1) - B(2,1),  A(2,2) - B(2,2),}
    }};
};

void P_Ht(const Mat3& P, const double H[2][3], double out[3][2]) {
    for (int i = 0; i < 3; ++i){
        for (int j = 0; j < 2; ++j) {
            out[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                out[i][j] += P(i, k) * H[j][k];
            }
        } 
    }
}

void H_P_Ht(const double H[2][3], const Mat3& P, double out[2][2]) {
    double PHt[3][2];
    P_Ht(P, H, PHt);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            out[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                out[i][j] += H[i][k] * PHt[k][j];
            }
        }
    }
}

double wrap_angle(double a) {
    while (a > M_PI) a -= 2 * M_PI;
    while (a < -M_PI) a += 2 * M_PI;
    return a;
}


State propagate(State s, Control u, double dt) {
    return {
        s.x + u.v * std::cos(s.theta) * dt,
        s.y + u.v * std::sin(s.theta) * dt,
        s.theta + u.omega * dt
    };
}

Meas measure(State s, Landmark lm) {
    double dx = lm.lx - s.x, dy = lm.ly - s.y;
    return {std::sqrt(dx*dx + dy*dy), std::atan2(dy, dx) - s.theta};  // dy first
}


void measure_jacobians(State s, Meas m, Landmark lm, Control u, double dt, double(&F)[3][3], double (&H)[2][3]) {
    F[0][0] = F[1][1] = F[2][2] = 1;
    F[0][2] = -u.v * std::sin(s.theta) * dt;
    F[1][2] = u.v * std::cos(s.theta) * dt;
    
    double dx = lm.lx - s.x, dy = lm.ly - s.y;
    H[0][2] = 0, H[1][2] = -1;
    H[0][0] = - dx/m.range, H[0][1] = -dy/m.range;
    H[1][0] = dy/(m.range * m.range), H[1][1] = -dx/(m.range * m.range);
}
