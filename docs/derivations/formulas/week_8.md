# Week 8 — Kalman Filter / Atomics & the Memory Model

## Math

**Core concepts:**
- The Kalman filter is Bayesian estimation for linear Gaussian systems
- State: x (what you want to know), Measurement: z (what sensors give you)
- Two steps every cycle:
  - **Predict:** propagate state and uncertainty through dynamics
  - **Update:** fuse measurement to reduce uncertainty

**Full linear KF equations:**

Predict:
```
x̂⁻ = F x̂         (state prediction — F is dynamics model)
P⁻ = F P Fᵀ + Q    (covariance prediction — Q is process noise)
```

Update:
```
ỹ = z − H x̂⁻                        (innovation — measurement residual)
S = H P⁻ Hᵀ + R                      (innovation covariance)
K = P⁻ Hᵀ S⁻¹                        (Kalman gain)
x̂ = x̂⁻ + K ỹ                        (state update)
P = (I − K H) P⁻                     (covariance update)
```

**Key intuitions:**
- K balances trust: if R is large (noisy sensor) → K small → trust prediction. If Q is large (uncertain dynamics) → P grows → K large → trust measurement.
- Innovation ỹ should be zero-mean with covariance S if the filter is calibrated
- P must stay symmetric positive definite — use Joseph form for numerical stability: `P = (I−KH) P⁻ (I−KH)ᵀ + K R Kᵀ`

**Derive (paper, 45 min):**
1. Predict step from marginalization of joint Gaussian (Week 7)
2. Update step from conditioning of joint Gaussian (Week 7)
3. Show that K = P⁻Hᵀ(HP⁻Hᵀ+R)⁻¹ minimizes the posterior covariance trace

## C++

**Atomics and the memory model:**
- `std::atomic<T>` — lock-free read/modify/write for small types (int, double, pointer)
- Memory orderings: `seq_cst` (default, safest, slowest), `acquire/release` (sufficient for most patterns), `relaxed` (no ordering guarantees)
- Why "works on x86" isn't correctness — x86 is strongly ordered, ARM is not
- Compare-and-swap: `atomic.compare_exchange_strong(expected, desired)`
- Spinlock from CAS — build one, understand why you almost never want one
- **SPSC ring buffer with acquire/release** — the canonical lock-free structure

**Exercises:**
1. Build a spinlock from `std::atomic_flag`. Test with two threads incrementing a counter. Compare performance against `std::mutex`.
2. Build a single-producer single-consumer (SPSC) lock-free ring buffer using `std::atomic<int>` for head/tail with acquire/release ordering.

## Project: Real-Time Kalman Tracker

**Architecture:**
- Thread 1 (sensor): simulates noisy 2D position measurements of a vehicle moving with constant velocity, pushes to SPSC queue at 100 Hz
- Thread 2 (estimator): pops measurements, runs KF predict+update, logs estimate

**System model (constant velocity):**
```
State: x = [px, py, vx, vy]ᵀ
F = [1 0 dt 0; 0 1 0 dt; 0 0 1 0; 0 0 0 1]
H = [1 0 0 0; 0 1 0 0]     (observe position only)
Q = process noise (tune it)
R = measurement noise covariance
```

**Implementation:**
- Use your MatX or a fixed-size 4×4 matrix for state/covariance
- SPSC queue from exercise 2 — no mutex on the hot path
- Log: timestamp, true state, measurement, estimate, innovation
- Dump to CSV for plotting

**Validation:**
- NEES (Normalized Estimation Error Squared): `(x_true − x̂)ᵀ P⁻¹ (x_true − x̂)` should average ≈ state dimension (4)
- Innovation should be zero-mean with covariance ≈ S
- TSan clean

## Done =
- [ ] KF derivation on paper (predict from marginalization, update from conditioning)
- [ ] Spinlock exercise
- [ ] SPSC ring buffer
- [ ] KF tracker: sensor thread → SPSC → estimator thread
- [ ] TSan clean
- [ ] CSV log with truth/measurement/estimate
- [ ] NEES or innovation check reported in README