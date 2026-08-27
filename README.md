# simcore

Self-directed HPC and simulation bootcamp: applied math + modern C++, in preparation for Georgia Tech MSCS (Computing Systems specialization).

**Cadence:** math derivations on paper, C++ experiments, weekly mini-project, CI (build + tests under ASan/UBSan) on every push.

## Progress

| Week | Topic | Project | Status |
|------|-------|---------|--------|
| 1 | Linear algebra as geometry, C++ object model | `vec3`/`mat3` library | done |
| 2 | Rotations, SO(3), quaternions, RAII and lifetime | `quat` + attitude toolkit | done |
| 3 | Jacobians, move semantics, smart pointers | numerical differentiation | done |
| 4 | ODE integration, templates and concepts | integrator suite | done |
| 5 | Linear solvers, STL internals | CSR sparse matrix + CG | done |
| 6 | PDEs and stability, memory hierarchy | cache-blocked heat solver | done |
| 7 | Probability for estimation, threads | multithreaded Monte Carlo | done |
| 8 | Kalman filter, atomics and memory model | lock-free KF tracker | done |
| 9 | EKF/UKF, thread pools | EKF localization | done |
| 10 | Nonlinear least squares, SIMD | AVX kernels + Gauss-Newton | done |
| 11 | Rigid body dynamics, profiling and allocators | rigid-body sim core | done |
| 12 | Capstone | `microphys` engine + live EKF | done |

## Architecture

See [docs/architecture.md](docs/architecture.md) for ownership, threading, and memory models.

## Layout

```
math/     header-only math library (built from scratch, no Eigen until Week 10)
sim/      simulation projects
tests/    doctest suites; every tests/test_*.cpp auto-registers with CTest
docs/     weekly formulas, architecture note
scratch/  standalone experiments and benchmarks
```

## Build

```
cmake -B build && cmake --build build && ctest --test-dir build
```

Debug builds compile with `-Wall -Wextra -Wpedantic -fsanitize=address,undefined`.

## Findings

**Week 2 -- Quat vs Mat3 (1M chained rotations, -O3):**
Quaternions trade marginal compose speed for 56% less storage, trivial renormalization, and slerp, which is why engines use them for orientation state. Quat norm drifted to 1.0000000000428 after 1M multiplies; one `normalized()` call fixes it. Quat: 4.02ms, Mat3: 3.03ms.

**Week 3 -- Numerical vs Analytic Jacobian (2-link arm):**
Central-difference Jacobian matches analytic to ~1e-7. Singularity confirmed at theta_2 = 0 (det(J) = 0, arm fully extended, end-effector loses one DOF).

**Week 4 -- Integrator Energy Drift (spring-mass, dt=0.01, 1000 steps):**
Explicit Euler gains energy (+10.5%), semi-implicit Euler oscillates (-0.5%), RK4 conserves to display precision. Semi-implicit is the physics-engine sweet spot: Euler's cost, bounded energy error.

**Week 5 -- CG Solver on 2D Laplace:**
CG iteration count grows linearly with grid size N (proportional to sqrt(kappa)). Preconditioners (not implemented) would flatten this.

**Week 6 -- 2D Heat Equation (N=1000, 100 steps, r=0.24):**
Three implementations (naive vector-of-vectors, flat array, cache-blocked) produce identical results. Stability bound dt <= dx^2/(4*alpha) confirmed empirically: r=0.24 stable, r=0.26 explodes.

**Week 7 -- Multithreaded Monte Carlo (10M samples, 2-link arm):**
Shared mutex: 5x slower than single-threaded (contention). Per-thread accumulators: near-linear speedup. At sigma=0.1 empirical covariance matches J*Sigma*J^T; at sigma=0.5 linearization breaks down, motivating nonlinear estimation (EKF).

**Week 8 -- Kalman Tracker (constant velocity, 500 steps, dt=0.01):**
Sensor thread to SPSC lock-free queue to estimator thread. No mutex on the hot path. Filter converges from zero initial guess within ~50 steps. Final estimate error < 0.1 at t=5s.

**Week 9 -- EKF Localization (unicycle, 4 landmarks, 500 steps):**
Converges from 2m position error + 0.5 rad heading error within ~50 steps. Final error < 0.07m position, < 0.01 rad heading. 10x R increase has minimal impact with 4 landmarks.

**Week 10 -- Optimization and SIMD:**
AVX2 dot product: 4.6x speedup at L1-resident sizes (N=1024), 2x at N=1M (memory-bound). Gauss-Newton with LM damping converges from (3, 0.5, 0.5) to (4.99, 0.30, 0.98) in 20 iterations against ground truth (5, 0.3, 1). Residual noise floor at sigma^2 * M/2.

**Week 11 -- Rigid Body Dynamics and Allocators:**
Arena allocator 6x faster than default new/delete for per-frame contact data (100K contacts x 100 frames). Semi-implicit Euler integration, sphere-plane bounce (e=0.8, settles correctly), sphere-sphere impulse collision (momentum conserved, energy loss matches restitution). Quaternion norm stable at 1.0 over 20K steps with per-step renormalization.

**Week 12 -- Capstone (microphys):**
100 rigid bodies + 10K particles, spatial hash broadphase, body-body and body-particle impulse collisions with angular response, EKF observer via SPSC lock-free queue. 0.9ms/step (1,100 Hz), 18x under 60 Hz target. Single-threaded outperforms 4-thread pool at this scale (0.033 vs 0.087ms) due to task submission overhead.

## Built With

C++20, GCC 13, CMake, doctest, AVX2 intrinsics, std::jthread, std::pmr, SPSC lock-free queue (hand-built)