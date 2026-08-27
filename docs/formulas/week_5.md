# Formula Sheet — Week 5: Linear Solvers, Sparse Matrices, STL

---

## Solving Ax = b

**LU decomposition:** factor A = LU (or PA = LU with pivoting)
```
Solve Ly = b  (forward substitution)
Solve Ux = y  (back substitution)
```
Cost: O(n³) factorization, O(n²) per solve. Factor once, solve many right-hand sides.

**Partial pivoting:** swap rows to put the largest entry on the diagonal before each elimination step. Without it, small pivots amplify rounding error catastrophically.

**Cholesky (SPD matrices only):** A = LLᵀ
```
Half the cost of LU, numerically stable without pivoting.
SPD = symmetric + all eigenvalues > 0.
Arises in: AᵀA (normal equations), covariance matrices, stiffness matrices.
```

---

## Condition Number

```
κ(A) = ‖A‖ · ‖A⁻¹‖ = |λ_max| / |λ_min|    (for symmetric A)
```

Error amplification bound:
```
‖δx‖/‖x‖ ≤ κ(A) · ‖δb‖/‖b‖
```

- κ ≈ 1: well-conditioned, errors don't amplify
- κ >> 1: ill-conditioned, small input errors → large output errors
- κ = ∞: singular

---

## CSR (Compressed Sparse Row)

For an m×n matrix with nnz nonzeros:
```
values[nnz]:      nonzero entries, row by row
col_idx[nnz]:     column index of each entry
row_ptr[m+1]:     values[row_ptr[i]..row_ptr[i+1]] are row i's entries
```

Sparse matrix-vector product `y = A*x`:
```
for i in 0..m:
    y[i] = 0
    for k in row_ptr[i]..row_ptr[i+1]:
        y[i] += values[k] * x[col_idx[k]]
```

---

## Conjugate Gradient (CG)

Solves Ax = b where A is SPD. Only needs mat-vec products (never forms A⁻¹).

```
r₀ = b - A·x₀
p₀ = r₀
for k = 0, 1, 2, ...:
    α_k = (rₖᵀrₖ) / (pₖᵀA·pₖ)
    x_{k+1} = xₖ + αₖ pₖ
    r_{k+1} = rₖ - αₖ A·pₖ
    if ‖r_{k+1}‖ < tol: converged
    β_k = (r_{k+1}ᵀr_{k+1}) / (rₖᵀrₖ)
    p_{k+1} = r_{k+1} + βₖ pₖ
```

- Converges in at most n iterations (exact arithmetic)
- Practical convergence depends on κ(A): iterations ∝ √κ
- No matrix storage needed — just a function that computes A·x

---

## 5-Point Laplace Stencil (2D)

Grid point (i,j), spacing h:
```
-u(i-1,j) - u(i,j-1) + 4u(i,j) - u(i+1,j) - u(i,j+1) = 0
```
Center = 4, four neighbors = −1. Yields an SPD system → CG applies.

For N×N interior grid: matrix is N²×N² but very sparse (≤ 5 entries per row).

---

## STL Internals

**`std::vector<T>`:**
```
Contiguous array. size ≤ capacity.
push_back: if size == capacity → allocate 2× capacity, move all elements, free old.
reserve(n): pre-allocate capacity ≥ n, no reallocation until size > n.
resize(n): changes size, default-constructs new elements.
```

**Iterator invalidation (vector):**
- push_back (realloc): ALL iterators, pointers, references invalidated
- push_back (no realloc): only end() invalidated
- insert/erase: everything at or after the point

**`std::span<T>` (C++20):**
```cpp
void process(std::span<double> data);    // non-owning view, zero copy
vector<double> v = {1,2,3,4,5};
process(v);                               // implicit conversion
process({v.data() + 1, 3});              // subrange: {2,3,4}
```

**`vector<vector<double>>` vs flat array:**
```
vector<vector<double>>: each row = separate heap allocation, pointer chasing, cache misses
vector<double> flat[rows*cols]: one allocation, contiguous, prefetcher-friendly
```
Always flatten for performance. MatX does this.