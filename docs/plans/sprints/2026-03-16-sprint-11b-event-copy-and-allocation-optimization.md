# Sprint 11B: Event Copy And Allocation Optimization Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** reduce the dominant post-SIMD cost center in `iojournal` by cutting event-copy allocation churn and redaction replacement churn, then refresh the release-facing benchmark and profiler evidence.

**Architecture:** keep the RC feature surface fixed. This sprint may change only internal event-copy ownership, internal redaction replacement strategy, and the evidence pack needed to validate the change.

**Tech Stack:** Podman, CMake presets, Unity, `hyperfine`, `uftrace`, `valgrind --tool=callgrind`, `PVS-Studio`, `CodeChecker`, `cppcheck`, C23.

**Current status:** Complete. The current local `clang-perf` rerun is [`20260316-160705`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-160705), and the refreshed profiler pack is [`20260316-164059`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-164059) plus `uftrace` companions [`20260316-164602`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-164602), [`20260316-164658`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-164658), and [`20260316-164607`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-164607). Sprint 11B now hands off to Sprint 12.

---

## Task 1: Add failure-first coverage for packed event-copy ownership

**Files:**
- Modify: `tests/unit/test_event.c`
- Modify: `tests/unit/CMakeLists.txt`

- [x] **Step 1: Add failing tests for packed ownership**

Expected coverage:
- copied event strings remain valid after redaction
- copied attribute keys and values remain independent from input memory
- disposal can free packed event-copy state without double-free behavior

- [x] **Step 2: Run targeted tests**

Run: `podman run --rm -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash -lc 'cmake --preset clang-debug >/tmp/s11b-red.log && cmake --build --preset clang-debug --target test_event && ctest --test-dir build/clang-debug --output-on-failure -R test_event'`
Expected: red test first, then green once implementation lands.

## Task 2: Replace multi-allocation event copy with packed ownership

**Files:**
- Modify: `src/core/ij_event.c`
- Modify: `src/ij_internal.h`

- [x] **Step 1: Add a packed event-copy allocation model**

Expected implementation:
- one allocation for the text arena
- one allocation for the attribute array at most
- no per-string `malloc` in the normal event-copy path

- [x] **Step 2: Keep redaction and validation behavior unchanged**

Expected:
- copied payload remains fully owned by `ij_event_copy_t`
- scalar and SIMD paths keep the same semantics
- `ij_event_copy_dispose()` stays correct for partial-failure cleanup

## Task 3: Remove per-redaction replacement allocation churn

**Files:**
- Modify: `src/filters/ij_redact.c`
- Modify: `src/core/ij_event.c`

- [x] **Step 1: Rework redaction replacement ownership**

Expected implementation:
- no standalone `malloc` per redacted string in the normal event-copy path
- `[REDACTED]` payload still appears exactly as before in emitted records

- [x] **Step 2: Verify behavior-sensitive tests**

Run: `podman run --rm -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash -lc 'ctest --test-dir build/clang-debug --output-on-failure -R \"test_event|test_filters|test_console_json|test_file_sink\"'`
Expected: formatting, redaction, and copy semantics remain green.

## Task 4: Refresh performance evidence and the sprint status

**Files:**
- Modify: `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- Modify: `docs/plans/comparison/RAW_ARTIFACTS.md`
- Modify: `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
- Modify: `docs/plans/sprints/README.md`
- Modify: `CHANGELOG.md`

- [x] **Step 1: Re-run the local baseline**

Run: `podman run --rm -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash -lc 'PRESET=clang-perf bash scripts/run-benchmarks.sh'`
Expected: fresh source-of-record local benchmark artifacts.

Current verified result:
- [`20260316-160705`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-160705)
- `enabled_console`: `1732.87 ns/op`
- `medium_message`: `1086.85 ns/op`
- `medium_message_with_metadata`: `1601.26 ns/op`
- `contention_mpsc`: `557.95 ns/op`
- `append_ndjson`: `1131.29 ns/op`

- [x] **Step 2: Re-run the profiler pack**

Run: `podman run --rm -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash -lc 'PRESET=clang-perf bash scripts/run-profiler-review.sh auto 3000'`
Run: `bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh uftrace bench_hot_path 2000 medium_message_with_metadata`
Run: `bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh uftrace bench_hot_path 2000 contention_mpsc`
Run: `bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh uftrace bench_file_sink 2000 append_ndjson`
Expected: refreshed callgrind, `hyperfine`, and `uftrace` artifacts for the post-allocation-optimization baseline.

- [x] **Step 3: Update the evidence docs**

Expected:
- comparison docs cite the post-optimization artifacts
- roadmap and master plan mark Sprint 11B complete only after the evidence refresh

- [x] **Step 4: Run the final scan for the sprint**

Run: `podman run --rm -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest python3 scripts/lint-docs.py`
Expected: `PASS`

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS: 14`, `FAIL: 0`, `SKIP: 0`

Current verified result:
- `PASS: 14`
- `FAIL: 0`
- `SKIP: 0`

## Execution Notes

- This sprint is release-blocking.
- Do not reopen `io_uring` or SIMD scope here; those decisions are already frozen for the RC line.
- Keep the work limited to event-copy, owned-string, and redaction-allocation churn.
- Do not widen queue topology, sink scope, or public API in this sprint.
- The first verified 11B slice also removes the synchronous `enqueue -> immediate dequeue` churn from `ij_logger_log()` and switches the console sink from `stdio` writes to `writev()`. These changes remain internal and do not widen the public API surface.
- Keep SIMD as an internal implementation detail only. Do not expose vector types in public headers, do not add `AVX-512` or `SVE/SVE2` work to this sprint, and keep runtime dispatch cached and isolated from the generic path.
