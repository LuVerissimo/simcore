#pragma once
#include "math/spsc_queue.hpp"
#include <fstream>
#include <iostream>
#include <vector>

struct Observation { float t, px, py; };

inline void run_ekf_offline(SPSCQueue<Observation, 2048>& obs_queue, const std::vector<std::pair<float, float>>& truth_log, float obs_dt) {

    std::ofstream log("microphys_ekf.csv");
    log << "t,true_px,true_py,meas_px,meas_py,est_px,est_py\n";

    // KF state: [px, py, vx, vy]
    double xk[4] = {0, 0, 0, 0};
    double P[4][4] = {{10,0,0,0},{0,10,0,0},{0,0,1,0},{0,0,0,1}};
    double Q[4][4] = {{0.001,0,0,0},{0,0.001,0,0},{0,0,0.01,0},{0,0,0,0.01}};
    double R_kf[2][2] = {{0.09, 0}, {0, 0.09}};  // sigma=0.3 → var=0.09

    Observation obs;
    int obs_idx = 0;
    while (obs_queue.pop(obs)) {
        auto [tx, ty] = truth_log[obs_idx++];

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
    std::cout << "EKF log: " << obs_idx << " observations\n";
}