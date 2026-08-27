# microphys Architecture

## Ownership Model
- Bodies: fixed-size SoA arrays (100), stack-allocated, lifetime = program
- Particles: heap-backed SoA vectors (10K), owned by main scope
- Contacts: arena-allocated per frame via pmr::monotonic_buffer_resource, bulk-freed each step
- SpatialHash: rebuilt each frame, std::unordered_map with heap-allocated buckets

## Threading Model
- ThreadPool (4 workers) parallelizes body and particle integration
- Bodies and particles are independent — no shared writes during integration
- Spatial hash, broadphase, and collision resolution are single-threaded (sequential dependency)
- At current scale, single-threaded (0.033ms) outperforms threaded (0.087ms) — task submission overhead exceeds compute

## Memory Model
- Arena allocator for per-frame contacts: pointer bump alloc, zero-cost dealloc via release()
- SoA layout for particles: contiguous float arrays enable auto-vectorization
- Bodies use SoA with fixed arrays — no heap after init
- SpatialHash is the main per-frame allocation cost (unordered_map buckets)

## EKF Observer
- Sensor produces noisy position observations of body 0 at 100 Hz via SPSC lock-free queue
- Offline KF pass after simulation: constant-velocity model, 4-state (px, py, vx, vy)
- No mutex on the observation path

## Performance
| Configuration | Per-step |
|---|---|
| 100 bodies only (no particles) | 0.033 ms |
| + 10K particles (no particle collisions) | 0.087 ms |
| + particle collisions in spatial hash | 0.913 ms |
| 60 Hz target | 16.6 ms |