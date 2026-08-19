# Formula Sheet — Week 10: Nonlinear Least Squares, SIMD

---

## Nonlinear Least Squares

**Problem:** minimize `½ ‖r(x)‖²` where r: Rⁿ → Rᵐ is the residual vector.

**Linearize:** `r(x + δ) ≈ r(x) + J·δ`

**Minimize ‖r + Jδ‖²:** take derivative w.r.t. δ, set to zero:
```
JᵀJ δ = −Jᵀr          (normal equations)
```

**Gauss-Newton iteration:**
```
for k = 0, 1, 2, ...:
    compute r(x_k) and J(x_k)
    solve JᵀJ δ = −Jᵀr for δ
    x_{k+1} = x_k + δ
    if ‖δ‖ < tol: converged
```

- JᵀJ is n×n SPD (when J has full column rank) → use Cholesky or CG
- Converges quadratically near the solution (like Newton's method)
- Fails when residuals are large or J is rank-deficient

---

## Levenberg-Marquardt

```
(JᵀJ + λI) δ = −Jᵀr
```
- λ large → δ ≈ −(1/λ)Jᵀr → gradient descent (safe, slow)
- λ small → δ ≈ GN step (fast, aggressive)
- Adaptive: decrease λ when step reduces cost, increase when it doesn't

---

## Gradient Descent

```
x_{k+1} = x_k − α ∇f(x_k)
```
- α = step size / learning rate
- Convergence rate depends on κ(H): steps ∝ κ for quadratic problems
- Ill-conditioned → zigzag → slow

---

## Exponential Model Fitting

Model: `y = a·exp(−b·x) + c`, parameters: `p = [a, b, c]`

Residual: `r_i = y_data_i − (a·exp(−b·x_i) + c)`

Jacobian (∂r_i/∂p):
```
∂r_i/∂a = −exp(−b·x_i)
∂r_i/∂b = a·x_i·exp(−b·x_i)
∂r_i/∂c = −1
```

---

## SIMD (AVX2)

**Register model:**
```
__m256d = 256-bit register holding 4 doubles
Operations execute on all 4 lanes simultaneously
```

**Key intrinsics:**
```cpp
__m256d a = _mm256_load_pd(ptr);         // load 4 aligned doubles
__m256d b = _mm256_loadu_pd(ptr);        // load 4 unaligned doubles
_mm256_store_pd(ptr, a);                 // store 4 aligned doubles
__m256d c = _mm256_mul_pd(a, b);         // 4 multiplies
__m256d d = _mm256_add_pd(a, b);         // 4 additions
__m256d e = _mm256_fmadd_pd(a, b, c);    // 4 fused multiply-adds (a*b + c)
__m256d s = _mm256_set1_pd(3.14);        // broadcast scalar to all 4 lanes
```

**Alignment:**
```cpp
alignas(32) double data[1024];           // 32-byte aligned for AVX
```

**Dot product (AVX2):**
```cpp
double simd_dot(const double* a, const double* b, int n) {
    __m256d sum = _mm256_setzero_pd();
    for (int i = 0; i < n; i += 4)
        sum = _mm256_fmadd_pd(_mm256_load_pd(a+i), _mm256_load_pd(b+i), sum);
    // horizontal sum of 4 lanes
    double tmp[4];
    _mm256_store_pd(tmp, sum);
    return tmp[0] + tmp[1] + tmp[2] + tmp[3];
}
```
Handle tail (n not divisible by 4) with scalar loop.

**Auto-vectorization checklist:**
- Simple loop body (arithmetic only)
- No function calls in the loop (or only inlined ones)
- No branches / early exits
- Unit stride access (a[i], not a[i*stride])
- No aliasing (`__restrict__` helps)
- Compile with `-O3 -march=native`
- Check: `-fopt-info-vec` (GCC) or `-Rpass=loop-vectorize` (Clang)

---

## Eigen Quick Reference

```cpp
#include <Eigen/Dense>
using namespace Eigen;

Matrix3d A;
Vector3d v;
MatrixXd M(rows, cols);      // dynamic size
auto result = A * v;          // expression templates — no temporary
auto AtA = A.transpose() * A;
auto x = AtA.llt().solve(Atb);  // Cholesky solve
```

Expression templates: Eigen fuses operations at compile time. `(A*B + C*D)` becomes one loop, no intermediates. This is why it's hard to beat.