# Formula Sheet — Week 6: PDEs, Stability, Memory Hierarchy

---

## Heat Equation (Parabolic)

Continuous: `∂u/∂t = α ∂²u/∂x²` (α = thermal diffusivity)

**1D explicit finite difference:**
```
u_i^{n+1} = u_i^n + r·(u_{i-1}^n − 2u_i^n + u_{i+1}^n)
where r = α·dt/dx²
```

**2D explicit:**
```
u_{i,j}^{n+1} = u_{i,j}^n + r·(u_{i-1,j} + u_{i+1,j} + u_{i,j-1} + u_{i,j+1} − 4u_{i,j})
where r = α·dt/dx²
```

**Stability (Von Neumann analysis):**
```
1D: dt ≤ dx² / (2α)       equivalently: r ≤ 0.5
2D: dt ≤ dx² / (4α)       equivalently: r ≤ 0.25
```
Above this → solution oscillates and grows unboundedly.

---

## Wave Equation (Hyperbolic)

Continuous: `∂²u/∂t² = c² ∂²u/∂x²`

**CFL condition:**
```
c · dt / dx ≤ 1
```
Physical meaning: a signal traveling at speed c must not cross more than one grid cell per timestep. Violation → numerical instability.

---

## Memory Hierarchy

```
Register        ~0.3 ns     ~few KB
L1 cache        ~1 ns       ~32 KB       64-byte cache lines
L2 cache        ~5 ns       ~256 KB
L3 cache        ~15 ns      ~8 MB        shared across cores
Main memory     ~60 ns      GBs
```

**Cache line:** 64 bytes = 8 doubles. CPU fetches entire lines. Accessing one double loads its 7 neighbors for free — IF you access them next (spatial locality).

---

## Access Patterns

**Row-major (C/C++ default):** `a[i][j]` with j in the inner loop → sequential memory access → cache hits.

**Column-major (Fortran, MATLAB):** `a[i][j]` with i in the inner loop → stride-N access → cache misses.

```cpp
// GOOD: inner loop over columns (row-major)
for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
        process(grid[i * N + j]);

// BAD: inner loop over rows (column-major access pattern)
for (int j = 0; j < N; ++j)
    for (int i = 0; i < N; ++i)
        process(grid[i * N + j]);
```

---

## Cache Blocking / Tiling

Process the grid in B×B blocks that fit in L1:
```cpp
for (int bi = 0; bi < N; bi += B)
    for (int bj = 0; bj < N; bj += B)
        for (int i = bi; i < min(bi+B, N); ++i)
            for (int j = bj; j < min(bj+B, N); ++j)
                process(grid[i * N + j]);
```

B chosen so B² doubles fit in L1: for 32 KB L1, B ≈ 64 (64×64×8 bytes = 32 KB).

---

## AoS vs SoA

**Array of Structs (AoS):**
```cpp
struct Particle { double x, y, z, vx, vy, vz; };
Particle p[N];    // x₀ y₀ z₀ vx₀ vy₀ vz₀ x₁ y₁ z₁ ... in memory
```
Good for: accessing all fields of one particle.

**Struct of Arrays (SoA):**
```cpp
struct Particles { double x[N], y[N], z[N], vx[N], vy[N], vz[N]; };
```
Good for: bulk operations on one field across all particles (vectorizable, cache-friendly).

---

## perf stat

```bash
perf stat -e cache-misses,cache-references,instructions,cycles ./program
```

Key ratio: `cache-misses / cache-references`. Lower = better cache utilization.