# simcore

Self-directed HPC & simulation bootcamp: applied math + modern C++, in preparation
for Georgia Tech MSCS (Computing Systems).

**Cadence:** math derivations → C++ experiments → weekly mini-project, with CI
(build + tests under ASan/UBSan) on every push.

## Progress

| Week | Topic | Project | Status |
|------|-------|---------|--------|
| 1 | Linear algebra as geometry · C++ object model | `vec3`/`mat3` library | done |
| 2 | Rotations, SO(3), quaternions · RAII & lifetime | `quat` + attitude toolkit | done |
| 3 | Jacobians · move semantics, smart pointers | numerical differentiation | done |
| 4 | ODE integration · templates & concepts | integrator suite | done |
| 5 | Linear solvers · STL internals | CSR sparse matrix + CG | done |
| 6 | PDEs & stability · memory hierarchy | cache-blocked heat solver | done |
| 7 | Probability for estimation · threads | MT Monte Carlo | done |
| 8 | Kalman filter · atomics & memory model | lock-free KF tracker | done |
| 9 | EKF/UKF · thread pools | EKF localization | done |
| 10 | Nonlinear least squares · SIMD | AVX kernels + Gauss-Newton | done |
| 11 | Rigid body dynamics · profiling & allocators | rigid-body sim core | done |
| 12 | Capstone | `microphys` engine + live EKF | in-progress |

## Layout
```
math/     header-only math library (built from scratch — no Eigen until Week 10)
sim/      simulation projects
tests/    doctest suites; every tests/test_*.cpp auto-registers with CTest
docs/     weekly curriculum + paper derivations
```

## Build
```
cmake -B build && cmake --build build && ctest --test-dir build
```
Debug builds compile with `-Wall -Wextra -Wpedantic -fsanitize=address,undefined`.


## Findings

**Week 2 — Quat vs Mat3 (1M chained rotations, -O3):**
Quaternions trade marginal compose speed for 56% less storage, trivial renormalization, and slerp — which is why engines use them for orientation state. 
Quat norm drifted to 1.0000000000428 after 1M multiplies; one `normalized()` call fixes it.
Results:

Quat: 4.0181 ms, norm drift: 1
1.0000000000428428
Mat3: 3.0337000000000001 ms, trace: 2.124758152590263


**Week 3 — Numerical vs Analytic Jacobian (2-link arm):**
Central-difference Jacobian matches analytic to ~1e-7. Singularity confirmed at θ₂ = 0 (det(J) ≈ 0 — arm fully extended, end-effector loses one DOF).


**Week 4 — Integrator Energy Drift (spring-mass, dt=0.01, 1000 steps):**
Explicit Euler gains energy (+10.5%), Semi-implicit Euler oscillates (−0.5%), RK4 conserves to display precision. Semi-implicit is the physics-engine sweet spot: Euler's cost, bounded energy error.


**Week 5 — CG Solver on 2D Laplace:**
CG iteration count grows linearly with grid size N (proportional to √κ). Preconditioners (not implemented) would flatten this.


**Week 6 — 2D Heat Equation (N=1000, 100 steps, r=0.24):**
Three implementations (naive vector<vector>, flat array, cache-blocked) all produce identical results.
Stability bound dt ≤ dx²/(4α) confirmed empirically: r=0.24 stable, r=0.26 explodes.


**Week 7 — Multithreaded Monte Carlo (10M samples, 2-link arm):**
Shared mutex: 5× slower than single-threaded (contention). Per-thread accumulators: near-linear speedup.
At σ=0.1 empirical covariance matches J·Σ·Jᵀ; at σ=0.5 linearization breaks down — motivates nonlinear estimation (EKF).


**Week 8 — Kalman Tracker (constant velocity, 500 steps, dt=0.01):**
Sensor thread → SPSC lock-free queue → estimator thread. No mutex on the hot path.
Filter converges from zero initial guess within ~50 steps. Final estimate error < 0.1 at t=5s.


**Week 9 — EKF Localization (unicycle, 4 landmarks, 500 steps):**
Converges from 2m position error + 0.5 rad heading error within ~50 steps. Final error < 0.07m position, < 0.01 rad heading. 10× R increase has minimal impact with 4 landmarks.

**Week 10 — **

*— SIMD Dot Product (AVX2 FMA)*: N=1024 (L1-resident): 4.6× speedup. N=1M (memory-bound): 2× speedup. SIMD gains are real but capped by memory bandwidth at scale.

*— SIMD + Gauss-Newton:*
AVX2 dot product: 4.6× at L1-resident sizes, 2× at 1M. GN with LM damping converges from (3, 0.5, 0.5) to (4.99, 0.30, 0.98) in 20 iterations against ground truth (5, 0.3, 1). Residual noise floor at σ²M/2.


**Week 11 — **

*— Arena vs new/delete (100K contacts × 100 frames)*: 6× faster than std::vector with default allocator. Zero fragmentation, one reset per frame.
*—  Rigid Body Simulator*: Semi-implicit Euler integration, sphere-plane bounce (e=0.8, settles correctly), sphere-sphere impulse collision (momentum conserved, energy loss matches restitution). Arena allocator 6× faster than default new/delete for per-frame contact data.