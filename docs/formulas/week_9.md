# Formula Sheet — Week 9: EKF, Unicycle Model, Thread Pools

---

## EKF (Extended Kalman Filter)

Same structure as linear KF, but dynamics and measurement are nonlinear functions.
Linearize with Jacobians at each step.

### Predict
```
x̂⁻ = f(x̂, u)                        nonlinear propagation
F = ∂f/∂x |_{x̂}                      Jacobian of dynamics
P⁻ = F P Fᵀ + Q
```

### Update
```
ỹ = z − h(x̂⁻)                       innovation (nonlinear h, not H·x)
H = ∂h/∂x |_{x̂⁻}                    Jacobian of measurement
S = H P⁻ Hᵀ + R
K = P⁻ Hᵀ S⁻¹
x̂ = x̂⁻ + K ỹ
P = (I − KH) P⁻
```

---

## Unicycle Model

State: `[x, y, θ]ᵀ`, Control: `[v, ω]ᵀ`

**Dynamics:**
```
x' = x + v cos(θ) dt
y' = y + v sin(θ) dt
θ' = θ + ω dt
```

**Jacobian F = ∂f/∂(x,y,θ):**
```
    [1   0   -v sin(θ) dt]
F = [0   1    v cos(θ) dt]
    [0   0    1           ]
```

---

## Range-Bearing Measurement

Landmark at `(lx, ly)`, robot at `(x, y, θ)`:

**Measurement model:**
```
range   = √((lx − x)² + (ly − y)²)
bearing = atan2(ly − y, lx − x) − θ
```

**Jacobian H = ∂h/∂(x,y,θ):**
```
Let dx = lx − x, dy = ly − y, r = √(dx² + dy²)

    [−dx/r    −dy/r     0 ]
H = [ dy/r²   −dx/r²   −1]
```

---

## UKF (Unscented Kalman Filter) — Conceptual

- Generate 2n+1 sigma points from current mean and covariance
- Propagate each through the nonlinear function
- Recover mean and covariance from the propagated points
- No Jacobians needed — works better for strong nonlinearity
- Same computational structure as EKF, just replaces linearization with sampling

---

## Thread Pool

```cpp
class ThreadPool {
    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;

public:
    ThreadPool(int n) {
        for (int i = 0; i < n; ++i)
            workers.emplace_back([this](std::stop_token st) {
                while (!st.stop_requested()) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(mtx);
                        cv.wait(lock, [&] { return !tasks.empty() || st.stop_requested(); });
                        if (st.stop_requested() && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
    }

    template<typename F>
    auto submit(F&& f) -> std::future<decltype(f())> {
        auto task = std::make_shared<std::packaged_task<decltype(f())()>>(std::forward<F>(f));
        auto fut = task->get_future();
        {
            std::lock_guard lock(mtx);
            tasks.push([task] { (*task)(); });
        }
        cv.notify_one();
        return fut;
    }

    ~ThreadPool() {
        for (auto& w : workers) w.request_stop();
        cv.notify_all();
    }
};
```

---

## std::async

```cpp
auto fut = std::async(std::launch::async, compute_jacobian, x);
// ... do other work ...
auto result = fut.get();  // blocks until done, rethrows exceptions
```

---

## Bearing Normalization

After computing bearing innovation, wrap to [−π, π]:
```cpp
while (angle > M_PI)  angle -= 2*M_PI;
while (angle < -M_PI) angle += 2*M_PI;
```
Forgetting this is the #1 EKF bearing bug.