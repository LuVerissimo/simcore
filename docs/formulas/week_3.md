# Formula Sheet — Week 3: Jacobians, Differentiation, Ownership

---

## Gradient

Scalar field f: Rⁿ → R:
```
∇f = (∂f/∂x₁, ∂f/∂x₂, ..., ∂f/∂xₙ)
```
- Points in direction of steepest ascent
- Magnitude = rate of steepest ascent
- Directional derivative along unit û: Dûf = ∇f · û

---

## Jacobian

Vector-valued map f: Rⁿ → Rᵐ:
```
       [ ∂f₁/∂x₁  ∂f₁/∂x₂  ...  ∂f₁/∂xₙ ]
  J =  [ ∂f₂/∂x₁  ∂f₂/∂x₂  ...  ∂f₂/∂xₙ ]     (m × n)
       [   ...                               ]
```
- Row i = gradient of fᵢ
- Column j = how all outputs change when xⱼ changes
- THE local linearization: f(x + δ) ≈ f(x) + J · δ
- If m = n: det(J) = 0 → singularity (not locally invertible)

---

## Hessian

Scalar field f: Rⁿ → R:
```
  H(i,j) = ∂²f / ∂xᵢ∂xⱼ        (n × n, symmetric)
```
- Positive definite H → local minimum
- Negative definite H → local maximum
- Indefinite → saddle point

---

## Chain Rule (matrix form)

h(x) = f(g(x)), where g: Rⁿ → Rᵏ, f: Rᵏ → Rᵐ:
```
  J_h = J_f · J_g        (m×k · k×n = m×n)
```

---

## 2-Link Planar Arm

Forward kinematics (joint angles → end-effector position):
```
  px = L₁ cos θ₁ + L₂ cos(θ₁ + θ₂)
  py = L₁ sin θ₁ + L₂ sin(θ₁ + θ₂)
```

Jacobian (2×2):
```
       [ -L₁ sinθ₁ - L₂ sin(θ₁+θ₂)    -L₂ sin(θ₁+θ₂) ]
  J =  [  L₁ cosθ₁ + L₂ cos(θ₁+θ₂)     L₂ cos(θ₁+θ₂) ]
```

Singularity: det(J) = L₁ L₂ sin θ₂ = 0 → θ₂ = 0 or π (arm fully extended or folded).

---

## Numerical Differentiation

**Forward difference:**
```
  f'(x) ≈ (f(x+h) - f(x)) / h          error: O(h)     best h ≈ √ε ≈ 1e-8
```

**Central difference:**
```
  f'(x) ≈ (f(x+h) - f(x-h)) / 2h       error: O(h²)    best h ≈ ∛ε ≈ 1e-5
```

**Numerical Jacobian (central):**
```
  J(:, j) = (f(x + h·eⱼ) - f(x - h·eⱼ)) / 2h
```
Perturb each input dimension, one column at a time.

---

## C++ Ownership

**Value categories:**
```
  lvalue: has a name, has an address (variables, dereferenced pointers)
  rvalue: temporary, no persistent address (literals, return values, std::move(x))
```

**`std::move`** = `static_cast<T&&>(x)` — cast, not operation. Enables move ctor/assignment.

**Smart pointers:**
```
  unique_ptr<T>    sole owner, move-only, zero overhead, delete on destruction
  shared_ptr<T>    reference counted, atomic refcount, shared ownership
  weak_ptr<T>      non-owning observer of shared_ptr, call lock() to access
  raw T*           non-owning observer, never call delete on it
```

**Ownership rules:**
- Factory returns `unique_ptr` → caller owns
- Function takes `unique_ptr` by value → takes ownership (caller must std::move)
- Function takes `const T&` or `T*` → borrows, does not own
- `shared_ptr` only when ownership is genuinely shared (rare)