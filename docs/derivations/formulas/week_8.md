# Formula Sheet — Week 8: Kalman Filter, Atomics, Lock-Free Structures

---

## Kalman Filter (Linear, Discrete)

**State:** x ∈ Rⁿ, **Measurement:** z ∈ Rᵐ

### Predict
```
x̂⁻ = F x̂                   state prediction
P⁻  = F P Fᵀ + Q            covariance prediction
```

### Update
```
ỹ = z − H x̂⁻                innovation (measurement residual)
S = H P⁻ Hᵀ + R              innovation covariance
K = P⁻ Hᵀ S⁻¹                Kalman gain
x̂ = x̂⁻ + K ỹ                state update
P = (I − KH) P⁻              covariance update
```

**Joseph form (numerically stable):**
```
P = (I − KH) P⁻ (I − KH)ᵀ + K R Kᵀ
```

### Matrices
```
F: state transition (n×n)     — how state evolves
H: observation (m×n)          — what sensors see
Q: process noise cov (n×n)    — uncertainty in dynamics
R: measurement noise cov (m×m) — uncertainty in sensors
K: Kalman gain (n×m)          — blending weight
P: state covariance (n×n)     — uncertainty in estimate
S: innovation covariance (m×m) — expected measurement spread
```

### Constant Velocity Model (2D)
```
State: x = [px, py, vx, vy]ᵀ

F = [1  0  dt  0 ]       H = [1  0  0  0]
    [0  1  0   dt]           [0  1  0  0]
    [0  0  1   0 ]
    [0  0  0   1 ]
```

### Validation
**NEES (Normalized Estimation Error Squared):**
```
ε = (x_true − x̂)ᵀ P⁻¹ (x_true − x̂)
E[ε] = n (state dimension)
```
If average NEES >> n → filter is overconfident. If << n → filter is conservative.

**Innovation check:** ỹ should be zero-mean with covariance ≈ S.

---

## Atomics

**`std::atomic<T>`** — hardware-level atomic operations, no mutex needed for small types.

```cpp
std::atomic<int> counter{0};
counter++;                          // atomic increment
counter.load();                     // atomic read
counter.store(42);                  // atomic write
```

**Compare-and-swap (CAS):**
```cpp
int expected = 0;
counter.compare_exchange_strong(expected, 1);
// if counter == expected: set to 1, return true
// else: load current into expected, return false
```

---

## Memory Orderings

```
seq_cst        total order, all threads agree on sequence     (default, safest)
acquire        reads after this see writes before the matching release
release        writes before this are visible after the matching acquire
relaxed        no ordering guarantees — only atomicity
```

**Acquire/release pair (the common pattern):**
```cpp
// Thread 1 (producer):
data = 42;                                          // non-atomic write
flag.store(true, std::memory_order_release);         // release: all prior writes visible

// Thread 2 (consumer):
while (!flag.load(std::memory_order_acquire)) {}     // acquire: sees all writes before release
assert(data == 42);                                  // guaranteed
```

---

## Spinlock from CAS

```cpp
class Spinlock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock()   { while (flag.test_and_set(std::memory_order_acquire)) {} }
    void unlock() { flag.clear(std::memory_order_release); }
};
```
Fast for very short critical sections. Bad for contention — wastes CPU cycles spinning.

---

## SPSC Ring Buffer (Lock-Free)

```cpp
template<typename T, int N>
class SPSCQueue {
    std::array<T, N> buf;
    std::atomic<int> head{0};   // written by producer
    std::atomic<int> tail{0};   // written by consumer

public:
    bool push(const T& val) {
        int h = head.load(std::memory_order_relaxed);
        int next = (h + 1) % N;
        if (next == tail.load(std::memory_order_acquire)) return false;  // full
        buf[h] = val;
        head.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& val) {
        int t = tail.load(std::memory_order_relaxed);
        if (t == head.load(std::memory_order_acquire)) return false;  // empty
        val = buf[t];
        tail.store((t + 1) % N, std::memory_order_release);
        return true;
    }
};
```

- One slot wasted (full = head+1 == tail) to distinguish full from empty
- No mutex, no CAS — just atomic load/store with acquire/release
- Only safe for exactly one producer and one consumer