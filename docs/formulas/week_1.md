# Formula Sheet — Week 1: Vectors, Matrices, Rotations

Reference card for `docs/`. Grows by one section per week. A formula you can't *derive* is a formula you don't own — this sheet is for speed, not for understanding.

---

## Vectors

**Dot product**
```
a·b = aₓbₓ + a_yb_y + a_zb_z = |a||b|cos θ
```
- `a·b = 0` ⇔ orthogonal
- `a·b > 0` ⇔ angle < 90°;  `a·b < 0` ⇔ angle > 90°
- `a·a = |a|²`

**Angle between vectors**
```
θ = arccos( a·b / (|a||b|) )
```

**Norm**
```
|a| = √(a·a)        (compare |a|² when possible — avoids the sqrt)
```

**Projection of a onto b**
```
proj_b(a) = (a·b / b·b) b          (vector)
comp_b(a) = a·b / |b|              (scalar length)
a⊥ = a − proj_b(a)                 (perpendicular remainder; a = proj + a⊥)
```

**Cross product** (3D only)
```
a×b = ( a_y b_z − a_z b_y,
        a_z b_x − a_x b_z,
        a_x b_y − a_y b_x )
```
- `|a×b| = |a||b|sin θ` = area of parallelogram
- Direction: right-hand rule; orthogonal to both inputs
- Anti-commutative: `a×b = −(b×a)`
- Not associative
- `a×a = 0`

**Scalar triple product** (signed volume)
```
a·(b×c) = det[a b c]
```
- `> 0` right-handed set, `< 0` left-handed, `= 0` coplanar
- Cyclic: `a·(b×c) = b·(c×a) = c·(a×b)`; any swap flips sign

---

## Matrices

**Matrix × vector** — linear combination of columns
```
Ax = x₁·(col₁ of A) + x₂·(col₂ of A) + x₃·(col₃ of A)
```
Column i of A = image of basis vector eᵢ.

**Matrix × matrix** — composition (right one acts first)
```
(AB)x = A(Bx)
col_j(AB) = A · col_j(B)
(AB)ᵢⱼ = Σₖ Aᵢₖ Bₖⱼ
```
- Associative, NOT commutative

**Transpose**
```
(Aᵀ)ᵢⱼ = Aⱼᵢ
(AB)ᵀ = BᵀAᵀ          (order reverses)
(A + B)ᵀ = Aᵀ + Bᵀ
(Aᵀ)ᵀ = A
```

**Determinant (3×3)** — via triple product of columns (or rows)
```
det A = a·(b×c)   where a, b, c are the columns
```
- `det(AB) = det(A)det(B)`
- `det(Aᵀ) = det(A)`
- `det = 0` ⇔ singular (columns linearly dependent, volume collapsed)
- |det| = volume scale factor of the map; sign = orientation preserved/flipped

---

## Rotations

**Elementary rotation matrices** (column convention, CCW positive, angle θ)
```
        [ 1    0       0    ]          [  cos θ   0   sin θ ]          [ cos θ  −sin θ   0 ]
R_x =   [ 0  cos θ  −sin θ ]    R_y =  [   0      1    0    ]    R_z = [ sin θ   cos θ   0 ]
        [ 0  sin θ   cos θ ]           [ −sin θ   0   cos θ ]          [  0       0      1 ]
```
Memory aid: build by columns — where do x̂, ŷ, ẑ go?

**Orthogonal / rotation matrix properties**
```
RᵀR = RRᵀ = I     ⇒     R⁻¹ = Rᵀ
det R = +1                     (reflections: RᵀR = I but det = −1)
|Rx| = |x|                     (lengths preserved)
(Rx)·(Ry) = x·y                (angles preserved)
```
Rows of R = old basis expressed in the new frame; that's *why* the transpose inverts.

**Composition order** (column-vector convention)
```
y = R₂ R₁ x        ⇒  R₁ applied first, then R₂
```

---

## Change of Basis

P = matrix whose **columns are the new basis vectors written in old coordinates**.
```
x_old = P x_new
x_new = P⁻¹ x_old
A_new = P⁻¹ A P            (same transform, new coordinates — "similarity")
```
If the new basis is orthonormal, P⁻¹ = Pᵀ and this is cheap.

---

## Simulation Toolbox

**Which side of a plane?** (plane through p₀ with normal n)
```
s = n·(q − p₀)        s > 0 front,  s < 0 back,  s = 0 on plane
```

**Velocity of a point on a rotating rigid body**
```
v = ω × r             (ω angular velocity, r position rel. to rotation axis/origin)
v·r = 0               (always tangent)
```

**Torque**
```
τ = r × F
```

**Right-handed orthonormal frame from forward f and approximate-up u**
```
f̂ = f/|f|
r̂ = (f̂ × u)/|f̂ × u|         (right)
û = r̂ × f̂                    (true up — recomputed, guaranteed orthonormal)
```

**Floating-point comparison** (never == on doubles)
```
approx_equal(a, b): |a − b| ≤ ε_abs  or  |a − b| ≤ ε_rel · max(|a|, |b|)
```