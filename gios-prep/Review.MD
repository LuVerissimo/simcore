# GIOS Review: Week 13 & 14

Concurrency and IPC, raw C, gcc 11.4 on WSL2.

## Week 13 — Threading, Synchronization, Sockets

**Race conditions.** Unsynchronized `global_counter++` across 8 threads loses
updates (`race.c`) — final count comes in under `NUM_THREADS * N` because the
increment isn't atomic. Wrapping the same increment in `pthread_mutex_lock`/
`unlock` fixes correctness at a measured throughput cost. Baseline for why
the bounded queue needs a lock at all.

**Bounded queue (producer/consumer).** Ring buffer + `pthread_mutex_t` +
two condvars (`not_full`, `not_empty`), classic signal-on-transition pattern.
Went through two broken versions before the correct one:
- `bq.c` / `bq2.c`: the buffer/index mutation (`buf[tail] = i`, `tail++`,
  `count++`) was written *inside* the `while (count == CAPACITY)` guard
  around `pthread_cond_wait`, instead of after it. Reads as correct at a
  glance since it's still inside the lock — the actual bug is that the
  mutation only runs when the wait loop's condition was true, i.e. only on
  a spurious-wakeup/full-buffer path, not on every iteration.
- `bq3.c`: mutation moved outside the wait loop, correct two-thread
  producer/consumer. Validated under ThreadSanitizer (`bq3_tsan` build).

**Memory safety.** `repro.c` — `recv()` writing into a string literal
(`"Hello, world!"`), i.e. into read-only memory. Deliberate crash/UB
demo, the kind of bug ASan or valgrind catches immediately and a happy-path
test never would.

**Socket skeleton.** Fork-per-connection TCP echo server (`server3.c`) +
line-oriented client (`client2.c`) using the standard `getaddrinfo` →
`socket`/`bind`/`listen`/`accept` (server) and `socket`/`connect` (client)
shape. Reused directly as the Week 14 Day 3 control-plane skeleton.

## Week 14 — POSIX Shared Memory IPC

**Day 1 — Segment lifecycle.** `shm_open`/`ftruncate`/`mmap`/`munmap`/
`shm_unlink` across two *separate binaries* (not threads). Ownership rule:
whoever `shm_open(O_CREAT)`s is the one that `shm_unlink`s. Ran the failure
first — mutex/condvar initialized without `PTHREAD_PROCESS_SHARED` on the
attr — to see the signature (hang / undefined behavior across processes)
before fixing it.

**Day 2 — Bounded queue ported into the segment.** Same queue as Week 13,
moved inside shm. Two changes forced by the process boundary:
- No raw pointers stored in the struct — each process's `mmap` return
  address differs, so anything pointer-based in shared state must be an
  offset from the segment base instead.
- Mutex/condvar created with `PTHREAD_PROCESS_SHARED` on the attr,
  living inside the mapped struct itself (not just process-local).

Validation gate: writer sends a file in fixed-size chunks through the
segment, reader reassembles, sha256 matches — and holds under a reader
slower than the writer, with no dropped or duplicated chunks (proves the
queue's backpressure actually blocks the writer rather than overwriting
unread slots).

**Day 3 — Socket + shm combined.** Small-scale GIOS project 3 shape: TCP
carries control only (file request, chunk count, done); the file's actual
bytes move through the Day 2 queue in shm. Server owns the segment
(creates, unlinks); client only opens and maps it.

**Day 4 — Error path discipline.** Rewrote shm/socket setup with
`goto cleanup`, one unwind path, each partial-init state (opened-not-
truncated, truncated-mmap-failed, mapped-condvar-init-failed) freeing
exactly what was acquired on that path — nothing more, nothing less.
Two real bugs caught in review, not hypothetical:
- A resource freed on the success path (`freeaddrinfo`) without clearing
  the "still need to free this" flag → every later failure branch
  double-freed it.
- The success path returning directly instead of going through
  `cleanup:` → skipped `close()` on the socket, meaning the *normal*
  exit leaked an fd, not just the error paths.

Valgrind clean across forced error branches on both binaries. gdb
walkthrough of a deliberate deadlock — non-recursive mutex locked twice
by the same thread, `futex_wait` with `expected=2` as the tell (locked +
a waiter). CI running build + valgrind pass on every push.

## Bugs worth remembering (recurring categories)

- **Guard-vs-body mixup around `pthread_cond_wait`**: state mutation
  written inside the `while` guard instead of after it (Week 13 `bq.c`/
  `bq2.c`). Looks locked and safe; only runs on the wrong condition.
- **Ownership-flag drift in goto-cleanup**: a flag meant to track "do I
  still owe this resource a free/close" not getting cleared when the
  resource is freed early on a success path — silent double-free.
- **Bypassing cleanup on success**: routing every failure through one
  cleanup label but letting the success path `return` directly defeats
  the whole point; the happy path needs to hit the same label.
- **`git filter-repo` strips the `origin` remote** as a safety measure —
  `git remote add origin` has to come *after* `filter-repo`, not before
  or instead of it.

## OSTEP chapters — still worth reading after Week 13/14

Locks and Condition Variables are now redundant against code already
written by hand. Keep: **Common Concurrency Problems** (deadlock
taxonomy — circular wait, hold-and-wait — maps directly onto the Day 4
gdb exercise), plus virtual memory, page tables, scheduling, and file
systems, none of which overlap with the concurrency/IPC work done here.