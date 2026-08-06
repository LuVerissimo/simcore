# Formula Sheet — Week 4: ODE Integration, Stability, Templates

---

## ODE Initial Value Problem

```
dy/dt = f(t, y),    y(t₀) = y₀
```

---

## Integrators

**Explicit Euler:**
```
y_{n+1} = y_n + h · f(t_n, y_n)
```
Local error: O(h²). Global error: O(h). Cheap, unstable for stiff systems.

**Semi-Implicit (Symplectic) Euler:**
```
v_{n+1} = v_n + h · a(x_n)         ← update velocity first
x_{n+1} = x_n + h · v_{n+1}        ← use NEW velocity
```
Same cost as explicit Euler. Energy oscillates but doesn't drift — used by most physics engines.

**RK4 (4th-order Runge-Kutta):**
```
k₁ = f(t_n,         y_n)
k₂ = f(t_n + h/2,   y_n + h/2 · k₁)
k₃ = f(t_n + h/2,   y_n + h/2 · k₂)
k₄ = f(t_n + h,     y_n + h · k₃)

y_{n+1} = y_n + (h/6)(k₁ + 2k₂ + 2k₃ + k₄)
```
Global error: O(h⁴). Four evaluations per step. The workhorse.

---

## Stability

Test equation: `y' = λy`, λ < 0 (stable ODE — solution decays).

**Explicit Euler:** `y_{n+1} = (1 + hλ) y_n`
Stable iff `|1 + hλ| < 1` → circle of radius 1 centered at (−1, 0) in the hλ-plane.

**Stiffness:** system has fast and slow modes (eigenvalues of widely different magnitude). Explicit methods must use dt dictated by the fastest mode even if accuracy only needs the slow one.

**Practical rule:** if explicit Euler blows up but the solution should be smooth and decaying → stiff system → need implicit or symplectic method.

---

## Harmonic Oscillator (energy test)

```
x' = v
v' = -(k/m)x
```
Total energy (should be constant): `E = ½mv² + ½kx²`

- Explicit Euler: E grows every step (gains energy)
- Implicit Euler: E shrinks every step (loses energy)
- Semi-implicit: E oscillates around true value (bounded)
- RK4: E nearly constant (very slow drift)

---

## Spring-Mass-Damper

```
x' = v
v' = -(k/m)x - (c/m)v
```
State vector: `y = (x, v)`, derivative: `f = (v, -(k/m)x - (c/m)v)`

Critical damping: c = 2√(km)

---

## Double Pendulum

State: `(θ₁, θ₂, ω₁, ω₂)`. Equations of motion are nonlinear and coupled — derive from Lagrangian or look up. Chaotic: sensitive to initial conditions and dt. Requires RK4 or better.

---

## C++ Templates

**Function template:**
```cpp
template<typename T>
T add(T a, T b) { return a + b; }
// compiler generates add<int>, add<double>, etc. on demand
```

**Concept (C++20):**
```cpp
template<typename S, typename State>
concept System = requires(S sys, double t, State y) {
    { sys(t, y) } -> std::convertible_to<State>;
};
```

**Cost comparison:**
```
std::function<State(double, State)>    heap alloc + virtual call (~30ns)
function pointer                       indirect call (~2ns)
template parameter                     inlined, zero overhead
```

**Templates must be in headers** — the compiler needs the full definition in every translation unit that uses them. No .cpp separation.