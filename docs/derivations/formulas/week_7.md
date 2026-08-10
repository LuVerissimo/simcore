# Formula Sheet — Week 7: Probability, Gaussians, Threading

---

## Random Variables

```
Expectation:    E[X] = μ
Variance:       Var(X) = E[(X − μ)²] = E[X²] − (E[X])²
Std deviation:  σ = √Var(X)
```

---

## Covariance Matrix

For random vector X = (X₁, ..., Xₙ):
```
Σᵢⱼ = E[(Xᵢ − μᵢ)(Xⱼ − μⱼ)] = Cov(Xᵢ, Xⱼ)
```
- Diagonal: Σᵢᵢ = Var(Xᵢ)
- Always symmetric: Σ = Σᵀ
- Always positive semi-definite: vᵀΣv ≥ 0 for all v
- Eigendecomposition Σ = QΛQᵀ: eigenvectors = principal directions, eigenvalues = variance along each

**Sample covariance (from N data points):**
```
μ̂ = (1/N) Σᵢ xᵢ
Σ̂ = (1/N) Σᵢ (xᵢ − μ̂)(xᵢ − μ̂)ᵀ
```

---

## Multivariate Gaussian

```
p(x) = (2π)^{-n/2} |Σ|^{-1/2} exp(−½ (x−μ)ᵀ Σ⁻¹ (x−μ))
```

Fully described by mean μ and covariance Σ. All marginals and conditionals are also Gaussian.

**Joint Gaussian partition:**
```
[x]     [μₓ]     [Σₓₓ  Σₓᵧ]
[y] ~ N([μᵧ],    [Σᵧₓ  Σᵧᵧ])
```

**Conditioning (x given y):**
```
x|y ~ N(μₓ + Σₓᵧ Σᵧᵧ⁻¹(y − μᵧ),  Σₓₓ − Σₓᵧ Σᵧᵧ⁻¹ Σᵧₓ)
```

**Marginalization:**
```
x ~ N(μₓ, Σₓₓ)        (just drop y)
```

---

## Bayes' Rule

```
posterior ∝ likelihood × prior
p(θ|x) = p(x|θ) p(θ) / p(x)
```

**1D Gaussian case:**
Prior: N(μ₀, σ₀²), Likelihood: N(x | θ, σ²)
```
Posterior mean:     μ_post = (σ² μ₀ + σ₀² x) / (σ² + σ₀²)
Posterior variance: σ_post² = (σ² σ₀²) / (σ² + σ₀²)
```
Weighted average — more certain source gets more weight.

---

## Linear Uncertainty Propagation

For y = f(x), x ~ N(μ, Σ), linearized:
```
E[y] ≈ f(μ)
Cov(y) ≈ J · Σ · Jᵀ       where J = Jacobian of f at μ
```
Exact for linear f. Approximate for nonlinear — breaks down for large Σ or strong nonlinearity.

---

## C++ Threading

**Basic thread:**
```cpp
#include <thread>
void work(int id) { /* ... */ }
std::jthread t(work, 42);      // auto-joins on destruction
```

**Mutex + lock_guard:**
```cpp
std::mutex mtx;
{
    std::lock_guard<std::mutex> lock(mtx);
    shared_data++;    // safe
}   // unlocked here — RAII
```

**scoped_lock (multiple mutexes, deadlock-free):**
```cpp
std::scoped_lock lock(mtx1, mtx2);    // locks both atomically
```

**Condition variable:**
```cpp
std::mutex mtx;
std::condition_variable cv;
bool ready = false;

// Waiting thread:
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock, [&]{ return ready; });    // handles spurious wakeups

// Signaling thread:
{
    std::lock_guard<std::mutex> lock(mtx);
    ready = true;
}
cv.notify_one();
```

**Data race → UB:**
Two threads, same memory, at least one write, no sync. Not "sometimes wrong" — undefined behavior. Anything can happen.

**False sharing:**
Two threads write to variables on the same 64-byte cache line → the line bounces between cores → performance collapse. Fix: pad to cache line boundaries.
```cpp
struct alignas(64) PaddedCounter { double value = 0; };
```

**ThreadSanitizer:**
```bash
g++ -std=c++20 -fsanitize=thread -g program.cpp -o program
```
Mandatory for all threaded code. Cannot combine with ASan.