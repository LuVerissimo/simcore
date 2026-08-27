# Formula Sheet — Week 2: Rotations, Quaternions, Eigenstructure

---

## Euler Angles (ZYX convention)

```
R = R_z(ψ) R_y(θ) R_x(φ)       (yaw, pitch, roll)
```
Gimbal lock: when pitch θ = ±90°, yaw and roll axes align → one DOF lost.

---

## Rodrigues' Rotation Formula

Rotate v about unit axis n by angle θ:
```
v' = v cosθ + (n × v) sinθ + n(n · v)(1 − cosθ)
```
Derivation: split v = v∥ + v⊥ where v∥ = n(n·v), v⊥ = v − v∥. Rotate v⊥ in its plane using (n × v) as the second basis.

---

## Quaternions

**Representation:** `q = w + xi + yj + zk` or `q = (w, v)` where `v = (x, y, z)`

**Basis rules:** `ij = k, jk = i, ki = j, ji = −k, kj = −i, ik = −j, i² = j² = k² = −1`

**Hamilton product:**
```
(a * b).w = a.w*b.w − a.x*b.x − a.y*b.y − a.z*b.z
(a * b).x = a.w*b.x + a.x*b.w + a.y*b.z − a.z*b.y
(a * b).y = a.w*b.y − a.x*b.z + a.y*b.w + a.z*b.x
(a * b).z = a.w*b.z + a.x*b.y − a.y*b.x + a.z*b.w
```

**Conjugate:** `q* = (w, −x, −y, −z)`

**Norm:** `|q| = √(w² + x² + y² + z²)`

**Inverse:** `q⁻¹ = q* / |q|²`  (for unit quaternions: `q⁻¹ = q*`)

**From axis-angle:** axis n (unit), angle θ:
```
q = (cos(θ/2),  n · sin(θ/2))
```

**Rotation of vector v:**
```
v' = q v q*          (embed v as pure quaternion (0, v))
```

Expanded (avoids constructing intermediate quaternions):
```
t = 2(q.xyz × v)
v' = v + w·t + q.xyz × t
```

**Composition:** `q_total = q₂ * q₁` (q₁ applied first, same convention as matrices)

**Quaternion → Rotation matrix:**
```
     [1−2(y²+z²)    2(xy−wz)     2(xz+wy)  ]
R =  [2(xy+wz)      1−2(x²+z²)   2(yz−wx)  ]
     [2(xz−wy)      2(yz+wx)     1−2(x²+y²) ]
```

**Slerp** (spherical linear interpolation):
```
θ = arccos(q₁ · q₂)               (4D dot product)
slerp(q₁, q₂, t) = q₁ sin((1−t)θ)/sinθ + q₂ sin(tθ)/sinθ
```
If `q₁ · q₂ < 0`, negate one (shortest path). If θ ≈ 0, fall back to lerp + normalize.

---

## Eigenvalues & Eigenvectors

**Definition:** `Av = λv`, v ≠ 0

**Characteristic polynomial:** `det(A − λI) = 0`

**2×2:**
```
λ = (tr(A) ± √(tr(A)² − 4 det(A))) / 2
```

**Symmetric matrices (A = Aᵀ):**
- All eigenvalues real
- Eigenvectors orthogonal (spectral theorem)
- A = QΛQᵀ where Q = eigenvector columns, Λ = diag(eigenvalues)
- Positive definite ⇔ all λ > 0

**Geometric meaning:** eigenvectors = invariant directions of the transform; eigenvalue = scale factor along that direction. Diagonalization = finding the basis where the transform is just scaling.

---

## C++ Lifetime Rules

**Destruction order:** reverse of construction (LIFO), always.

**RAII:** acquire in ctor, release in dtor. Stack unwinding (exceptions) guarantees dtor runs.

**Rule of zero:** if all members manage themselves (e.g., `vector`, `unique_ptr`), write no special members.

**Rule of three:** if you write any of {destructor, copy ctor, copy assignment}, write all three.

**Rule of five:** rule of three + move ctor + move assignment.

**Move semantics:**
- `T&&` = rvalue reference (binds to temporaries / `std::move`'d objects)
- `std::move(x)` = cast to rvalue, does NOT move — enables the move ctor/assignment to steal
- Moved-from state: valid but unspecified (safe to destroy or reassign)
- `noexcept` on moves lets `vector` use them during reallocation