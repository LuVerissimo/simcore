# simcore

Self-directed HPC & simulation bootcamp: applied math + modern C++, in preparation
for Georgia Tech OMSCS (Computing Systems specialization, Spring 2027).

**Cadence:** math derivations → C++ experiments → weekly mini-project, with CI
(build + tests under ASan/UBSan) on every push.

## Progress

| Week | Topic | Project | Status |
|------|-------|---------|--------|
| 1 | Linear algebra as geometry · C++ object model | `vec3`/`mat3` library | done |
| 2 | Rotations, SO(3), quaternions · RAII & lifetime | `quat` + attitude toolkit | in progress |
| 3 | Jacobians · move semantics, smart pointers | numerical differentiation | — |
| 4 | ODE integration · templates & concepts | integrator suite | — |
| 5 | Linear solvers · STL internals | CSR sparse matrix + CG | — |
| 6 | PDEs & stability · memory hierarchy | cache-blocked heat solver | — |
| 7 | Probability for estimation · threads | MT Monte Carlo | — |
| 8 | Kalman filter · atomics & memory model | lock-free KF tracker | — |
| 9 | EKF/UKF · thread pools | EKF localization | — |
| 10 | Nonlinear least squares · SIMD | AVX kernels + Gauss-Newton | — |
| 11 | Rigid body dynamics · profiling & allocators | rigid-body sim core | — |
| 12 | Capstone | `microphys` engine + live EKF | — |

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
