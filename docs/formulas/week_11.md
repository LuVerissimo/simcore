# Formula Sheet — Week 11: Rigid Body Dynamics, Allocators

---

## Rigid Body State

```
Position:           p ∈ R³
Orientation:        q (unit quaternion)
Linear velocity:    v ∈ R³
Angular velocity:   ω ∈ R³ (body frame or world frame — pick one, be consistent)
```

---

## Integration (Semi-Implicit Euler)

```
v_{n+1} = v_n + (F/m) · dt                      linear
ω_{n+1} = ω_n + I⁻¹(τ − ω × Iω) · dt          angular (Euler's equations)
p_{n+1} = p_n + v_{n+1} · dt                     position (uses NEW velocity)
q_{n+1} = q_n + ½ · (0, ω_{n+1}) · q_n · dt     orientation (quaternion derivative)
q_{n+1} = normalize(q_{n+1})                     re-normalize every step
```

---

## Inertia Tensor

**Uniform box** (mass m, width w, height h, depth d):
```
I = diag(m/12(h²+d²), m/12(w²+d²), m/12(w²+h²))
```

**Uniform sphere** (mass m, radius r):
```
I = diag(2mr²/5, 2mr²/5, 2mr²/5)
```

**Parallel axis theorem** (shift by displacement d from CM):
```
I_shifted = I_cm + m(|d|²·I₃ − d·dᵀ)
```

**World-frame inertia** (R = rotation matrix from body to world):
```
I_world = R · I_body · Rᵀ
```

---

## Euler's Rotation Equations

In body frame:
```
I·ω̇ = τ − ω × (I·ω)
```

The `ω × Iω` term (gyroscopic torque) couples axes and causes:
- Precession
- Nutation
- Tennis racket instability (intermediate axis)

**Tennis racket theorem:** for I₁ < I₂ < I₃, rotation about axis 2 (intermediate) is unstable. Linearize Euler's equations about each axis → eigenvalues are imaginary for axes 1,3 (stable oscillation) but real for axis 2 (exponential divergence).

---

## Collision Detection

**Sphere-sphere:**
```
d = |p₁ − p₂|
colliding if d < r₁ + r₂
normal n = (p₁ − p₂) / d
penetration = r₁ + r₂ − d
```

**Sphere-plane** (plane with normal n, distance d from origin):
```
signed_dist = p · n − d − r
colliding if signed_dist < 0
contact normal = n
```

---

## Impulse-Based Collision Response

**Relative velocity at contact:**
```
v_rel = (v₁ + ω₁ × r₁) − (v₂ + ω₂ × r₂)
```
where r₁, r₂ = vectors from each body's CM to the contact point.

**Impulse magnitude** (with restitution e):
```
j = -(1 + e)(v_rel · n) / (1/m₁ + 1/m₂ + n·(I₁⁻¹(r₁×n))×r₁ + n·(I₂⁻¹(r₂×n))×r₂)
```

**Sphere-plane simplification** (plane has infinite mass/inertia):
```
j = -(1 + e)(v_rel · n) / (1/m + n·(I⁻¹(r×n))×r)
```

**Apply impulse:**
```
v₁ += (j/m₁) · n
ω₁ += I₁⁻¹ · (r₁ × j·n)
v₂ -= (j/m₂) · n
ω₂ -= I₂⁻¹ · (r₂ × j·n)
```

---

## Allocators

**Arena (linear) allocator:**
```cpp
class Arena {
    char* buf;
    size_t offset = 0, capacity;
public:
    Arena(size_t cap) : buf(new char[cap]), capacity(cap) {}
    ~Arena() { delete[] buf; }

    void* alloc(size_t size, size_t align = 8) {
        offset = (offset + align - 1) & ~(align - 1);  // align
        void* ptr = buf + offset;
        offset += size;
        return ptr;
    }
    void reset() { offset = 0; }  // "free" everything instantly
};
```
- Allocation = pointer bump (< 1 ns)
- Deallocation = reset pointer (free)
- No individual frees — everything dies together
- Perfect for per-frame scratch data (contacts, broadphase pairs)

**`std::pmr` (standard arena):**
```cpp
#include <memory_resource>
char buffer[4096];
std::pmr::monotonic_buffer_resource pool(buffer, sizeof(buffer));
std::pmr::vector<Contact> contacts(&pool);
// contacts allocates from the stack buffer — no heap
```

**Pool allocator:**
- Fixed-size blocks, free list
- O(1) alloc: pop from free list
- O(1) dealloc: push to free list
- Good for same-size objects (particles, nodes)

---

## SoA Layout

```cpp
// AoS (Array of Structs)
struct Body { vec3 pos, vel; quat ori; vec3 omega; double mass; };
Body bodies[N];

// SoA (Struct of Arrays) — cache-friendly for bulk updates
struct Bodies {
    vec3 pos[N], vel[N];
    quat ori[N];
    vec3 omega[N];
    double mass[N];
};
```

SoA advantages: position update touches only `pos[]` and `vel[]` — contiguous, prefetcher-friendly, vectorizable.