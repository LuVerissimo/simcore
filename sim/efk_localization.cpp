#include <cmath>
#include <iostream>
#include <random>
#include "math/thread_pool.hpp"
#include <fstream>
#include <iostream>

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
struct Mat2_3 {
    double m[2][3]{};
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
        {A(0,0), A(1,0), A(2,0)},
        {A(0,1), A(1,1), A(2,1)},
        {A(0,2), A(1,2), A(2,2)}
    }};
};

Mat2_3 transpose2_3(const Mat2_3& A) {
    return {{
        {A(0,0), A(1,0), A(2,0)},
        {A(0,1), A(1,1), A(2,1)},
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

void inv2x2(double a, double b, double c, double d,
            double& ai, double& bi, double& ci, double& di) {
    double det = a*d - b*c;
    ai = d/det; bi = -b/det; ci = -c/det; di = a/det;
}

int main() {
    const int N = 500;
    const double dt = 0.1;
    Control u{1.0,0.3}; //drive in a curve

    // 4 landmarks
    Landmark landmarks[] = {{5,5},{-5,5},{-5,-5},{5,-5}};

    State truth{0,0,0};

    State est{2,2,0.5}; //crappy initial guess
    Mat3 P{};
    P(0,0) = P(1,1) = P(2,2) = 1;

    Mat3 Q{};
    Q(0,0) = Q(1,1) = 0.001;

    double R[2][2] = {{1.0,0},{0,0.05}}; //range noise, bearing noise

    std::mt19937 gen(42);
    std::normal_distribution<double> noise_r(0, sqrt(0.1));
    std::normal_distribution<double> noise_b(0, sqrt(0.05));

    std::ofstream log("ekf_log.csv");
    log << "t,true_x,true_y,true_theta,est_x,est_y,est_theta\n";

    for (int i = 0; i < N; ++i) {
        truth = propagate(truth, u, dt);
        truth.theta = wrap_angle(truth.theta);


        // Predict
        est = propagate(est, u, dt);
        Mat3 F {};
        F(0,0) = F(1,1) = F(2,2) = 1;
        F(0,2) = -u.v * std::sin(est.theta) * dt;
        F(1,2) =  u.v * std::cos(est.theta) * dt;
        P = add3(mat3_mat3(mat3_mat3(F, P), transpose3(F)), Q);

        // Update (once per landmark)
        for (auto& lm : landmarks) {
            // True measurement + noise
            Meas z_true = measure(truth, lm);
            Meas z{z_true.range + noise_r(gen), z_true.bearing + noise_b(gen)};
            
            // Predicted measurement from est
            Meas z_pred = measure(est, lm);

            // Innovation (wrap bearing!)
            double y[2] = {z.range - z_pred.range, wrap_angle(z.bearing - z_pred.bearing)};

            // H Jacobian at current estimate
            double dx = lm.lx - est.x, dy = lm.ly - est.y;
            double r = z_pred.range;
            double H[2][3] = {
                {-dx/r,        -dy/r,         0},
                { dy/(r*r),    -dx/(r*r),    -1}
            };

            // S = H P Hᵀ + R
            double S[2][2];
            H_P_Ht(H, P, S);
            S[0][0] += R[0][0]; S[1][1] += R[1][1];

            // S⁻¹
            double si00, si01, si10, si11;
            inv2x2(S[0][0], S[0][1], S[1][0], S[1][1], si00, si01, si10, si11);

            // K = P Hᵀ S⁻¹  (3×2)
            double PHt[3][2];
            P_Ht(P, H, PHt);
            double K[3][2];
            for (int j = 0; j < 3; ++j) {
                K[j][0] = PHt[j][0]*si00 + PHt[j][1]*si10;
                K[j][1] = PHt[j][0]*si01 + PHt[j][1]*si11;
            }

            //state update
            est.x     += K[0][0]*y[0] + K[0][1]*y[1];
            est.y     += K[1][0]*y[0] + K[1][1]*y[1];
            est.theta += K[2][0]*y[0] + K[2][1]*y[1];
            est.theta = wrap_angle(est.theta);

            // P = (I - KH) P
            Mat3 KH{};
            for (int j = 0; j < 3; ++j)
                for (int c = 0; c < 3; ++c)
                    KH(j, c) = K[j][0]*H[0][c] + K[j][1]*H[1][c];
            
            Mat3 I{};
            I(0, 0) = I(1,1) = I(2,2) = 1.0;
            // P = (I - K*H) * P
            P = mat3_mat3(sub3(I, KH), P);
        }
        log << i*dt << "," << truth.x << "," << truth.y << "," << truth.theta << ","
        << est.x << "," << est.y << "," << est.theta << "\n";
    }
    std::cout << "Final truth:    " << truth.x << " " << truth.y << " " << truth.theta << "\n";
    std::cout << "Final estimate: " << est.x << " " << est.y << " " << est.theta << "\n";
    std::cout << "Error:          " << std::abs(truth.x-est.x) << " "
    << std::abs(truth.y-est.y) << " " << std::abs(wrap_angle(truth.theta-est.theta)) << "\n";
}