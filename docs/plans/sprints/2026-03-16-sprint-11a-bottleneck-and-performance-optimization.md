# Sprint 11A: Bottleneck And Performance Optimization Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** confirm the dominant runtime bottlenecks in `iojournal`, introduce only the optimizations that are proven relevant, and refresh the release-facing evidence before the final `v0.1.0-rc.1` publication gate.

**Architecture:** treat optimization as a measured program, not as a blind tuning pass. First lock the container and benchmark environment for `io_uring` and ISA-specific experiments, then confirm bottlenecks from the existing shared-scenario profiler set, then evaluate `io_uring` and SIMD relevance separately, and only then implement the profitable optimizations behind preserved scalar and synchronous fallbacks.

**Tech Stack:** Podman, CMake presets, `hyperfine`, `uftrace`, `valgrind --tool=callgrind`, `gdb`, `PVS-Studio`, `CodeChecker`, `cppcheck`, C23, optional `io_uring`, scalar plus `AVX2` and `NEON` fast paths.

**Status:** complete. Task 5 was intentionally not expanded because the file-sink `io_uring` path was rejected for the RC-active path, so the sprint closed on the refreshed post-SIMD evidence pack instead.

---

## Chunk 1: Environment And Measurement Gates

### Task 1: Add the official Podman launch mode for `io_uring` perf lanes

**Files:**
- Modify: `deploy/podman/Containerfile`
- Modify: `AGENTS.md`
- Modify: `CLAUDE.md`
- Modify: `CODEX.md`
- Modify: `docs/en/04-tooling-and-agent-workflow.md`
- Modify: `docs/ru/04-tooling-and-agent-workflow.md`
- Modify: `docs/testing/PROFILER_WORKFLOW.md`
- Modify: `scripts/run-profiler-review.sh`
- Create: `scripts/run-podman-perf-lane.sh`

- [ ] **Step 1: Write the failing environment check**

Run: `podman run --rm localhost/iojournal-dev:latest python3 -c 'import ctypes,os; print(ctypes.CDLL(None,use_errno=True).syscall(425,2,0))'`
Expected: failure or `ENOSYS` under the default container launch mode.

- [ ] **Step 2: Add the dedicated perf launch wrapper**

Expected implementation:
- the wrapper runs Podman with the explicit security mode required for `io_uring` study
- the wrapper is documented as perf-lane only, not as the default development mode

- [ ] **Step 3: Verify the `io_uring` lane**

Run: `bash scripts/run-podman-perf-lane.sh python3 -c 'import ctypes, os; ...'`
Expected: `io_uring_setup()` succeeds inside the official perf lane.

- [ ] **Step 4: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS: 14`, `FAIL: 0`, `SKIP: 0`

### Task 2: Freeze the bottleneck evidence baseline

**Files:**
- Modify: `docs/testing/BENCHMARK_METHODOLOGY.md`
- Modify: `docs/testing/PROFILER_WORKFLOW.md`
- Create: `docs/testing/PERF_OPTIMIZATION_GATE.md`
- Create: `docs/tmp/perf-analysis/README.md`

- [ ] **Step 1: Define the optimization gate**

Expected decisions:
- no optimization lands without a before/after measurement
- every optimization must name one benchmark row and one profiler artifact
- every optimization keeps the scalar baseline valid

- [ ] **Step 2: Freeze the scenario set used by the optimization sprint**

Expected shared scenarios:
- `disabled_level`
- `enabled_console`
- `medium_message`
- `medium_message_with_metadata`
- `contention_mpsc`
- `append_file`

- [ ] **Step 3: Verify the docs**

Run: `python3 scripts/lint-docs.py`
Expected: `PASS`

## Chunk 2: Bottleneck Confirmation

### Task 3: Produce the hotspot inventory from the current implementation

**Files:**
- Create: `docs/testing/PERF_HOTSPOT_INVENTORY.md`
- Modify: `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- Modify: `docs/plans/comparison/RAW_ARTIFACTS.md`

- [x] **Step 1: Re-run the release-facing local baseline**

Run: `PRESET=clang-perf bash scripts/run-benchmarks.sh`
Expected: one fresh local benchmark directory under `docs/tmp/benchmarks/`

- [x] **Step 2: Re-run the profiler pack**

Run: `bash scripts/build-uftrace-bench.sh`
Run: `bash scripts/run-profiler-review.sh auto bench_hot_path 3000`
Run: `bash scripts/run-profiler-review.sh uftrace bench_hot_path 2000 medium_message_with_metadata`
Run: `bash scripts/run-profiler-review.sh uftrace bench_hot_path 2000 contention_mpsc`
Run: `bash scripts/run-profiler-review.sh uftrace bench_file_sink 2000 append_ndjson`
Expected: fresh `callgrind`, `hyperfine`, and `uftrace` artifacts under `docs/tmp/profiling/`

- [x] **Step 3: Write the hotspot inventory**

Expected inventory sections:
- encode-time hotspots
- redaction hotspots
- event-copy hotspots
- queue and lock hotspots
- sink-write hotspots

- [x] **Step 4: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS: 14`, `FAIL: 0`, `SKIP: 0`

## Chunk 3: io_uring Relevance And Adoption

### Task 4: Prove or reject `io_uring` relevance for the file sink

**Files:**
- Create: `src/sinks/ij_file_sink_sync.c`
- Create: `src/sinks/ij_file_sink_uring.c`
- Create: `include/iojournal/iojournal_perf.h`
- Modify: `src/core/ij_logger.c`
- Modify: `src/ij_internal.h`
- Create: `bench/bench_file_sink_uring.c`
- Create: `tests/unit/test_file_sink_uring.c`
- Create: `docs/testing/IO_URING_RELEVANCE.md`

- [x] **Step 1: Write the failing unit test for backend selection**

Test: `tests/unit/test_file_sink_uring.c`
Expected failure: no `io_uring`-capable backend selection exists yet.

- [x] **Step 2: Add the synchronous and `io_uring` backend split**

Expected implementation:
- preserve the current synchronous file path as the default fallback
- add an explicit internal `io_uring` backend path
- keep public API expansion minimal and RC-safe

- [x] **Step 3: Add the dedicated benchmark**

Run: `cmake --build --preset clang-perf --target bench_file_sink bench_file_sink_uring`
Expected: both sync and `io_uring` file sink lanes can be measured side by side.

- [x] **Step 4: Measure relevance**

Run: `bash scripts/run-podman-perf-lane.sh bash -lc 'cmake --build --preset clang-perf --target bench_file_sink bench_file_sink_uring && ./build/clang-perf/bench/bench_file_sink_uring 50000 --tsv'`
Expected: one explicit result row for `io_uring` file append

- [x] **Step 5: Make the accept or reject decision**

Expected rule:
- if `io_uring` does not materially improve the target file scenario, keep the implementation out of the RC path and record the rejection
- if it does improve the target scenario materially without breaking analyzer and test gates, keep it in the RC path behind the documented launch mode and fallback

- [x] **Step 6: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS: 14`, `FAIL: 0`, `SKIP: 0`

### Task 5: Evaluate network-sink `io_uring` only if the file path is accepted or still clearly syscall-bound

**Files:**
- Create: `bench/bench_syslog_tcp_uring.c`
- Modify: `docs/testing/IO_URING_RELEVANCE.md`
- Modify: `docs/plans/comparison/PERFORMANCE_RESULTS.md`

- [ ] **Step 1: Write the failing benchmark harness**

Expected: no `io_uring` network benchmark exists yet.

- [ ] **Step 2: Add the narrow network `io_uring` experiment**

Expected scope:
- syslog TCP only
- no UDP `io_uring` work unless TCP numbers justify further expansion

- [ ] **Step 3: Measure and decide**

Run: `bash scripts/run-podman-perf-lane.sh ...`
Expected: one explicit decision in `docs/testing/IO_URING_RELEVANCE.md`

- [ ] **Step 4: Keep YAGNI boundaries**

Expected:
- no TLS
- no OTLP
- no broad multi-sink async runtime introduction in this sprint

## Chunk 4: SIMD Relevance And Adoption

### Task 6: Prove or reject SIMD targets

**Files:**
- Create: `src/encoders/ij_simd_dispatch.h`
- Create: `src/encoders/ij_json_escape_scalar.c`
- Create: `src/encoders/ij_json_escape_x86_avx2.c`
- Create: `src/encoders/ij_json_escape_arm_neon.c`
- Create: `src/filters/ij_redact_match_scalar.c`
- Create: `src/filters/ij_redact_match_x86_avx2.c`
- Create: `src/filters/ij_redact_match_arm_neon.c`
- Create: `src/core/ij_utf8_scalar.c`
- Create: `src/core/ij_utf8_x86_avx2.c`
- Create: `src/core/ij_utf8_arm_neon.c`
- Create: `bench/bench_simd_scan.c`
- Create: `tests/unit/test_simd_dispatch.c`
- Create: `docs/testing/SIMD_RELEVANCE.md`

- [x] **Step 1: Write the failing dispatch test**

Test: `tests/unit/test_simd_dispatch.c`
Expected failure: no scalar plus ISA-specific dispatch layer exists yet.

- [x] **Step 2: Add the scalar baseline units**

Expected:
- keep the current scalar logic correct and testable in isolated units
- no behavior change before ISA-specific fast paths are added

- [x] **Step 3: Add the `AVX2` and `NEON` fast paths**

Expected rules:
- `x86_64`: `AVX2` only
- `arm64`: `NEON` only
- no `AVX-512`
- no `SVE`
- no `SVE2`

- [x] **Step 4: Add runtime dispatch**

Expected:
- compile-time and runtime feature detection remain explicit
- unsupported CPUs continue to use the scalar path

- [x] **Step 5: Measure relevance**

Run: `cmake --build --preset clang-perf --target bench_simd_scan`
Run: `./build/clang-perf/bench/bench_simd_scan --tsv`
Expected: before/after evidence for JSON escaping, redact-key matching, and UTF-8 validation

- [x] **Step 6: Make the accept or reject decision per target**

Expected:
- each target gets an explicit keep or drop verdict in `docs/testing/SIMD_RELEVANCE.md`
- unprofitable ISA-specific code does not stay in the RC path

- [x] **Step 7: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS: 14`, `FAIL: 0`, `SKIP: 0`

## Chunk 5: Evidence Refresh And RC Handoff

### Task 7: Refresh the comparison pack after optimization

**Files:**
- Modify: `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- Modify: `docs/plans/comparison/RAW_ARTIFACTS.md`
- Modify: `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
- Modify: `docs/plans/sprints/README.md`
- Modify: `CHANGELOG.md`

- [x] **Step 1: Re-run the local baseline**

Run: `PRESET=clang-perf bash scripts/run-benchmarks.sh`
Expected: fresh source-of-record local benchmark artifacts

- [x] **Step 2: Re-run Tier 1 comparison**

Run: `bash scripts/run-tier1-benchmarks.sh`
Expected: fresh Tier 1 competitor artifacts

- [x] **Step 3: Re-run profiling**

Run: `bash scripts/run-profiler-review.sh auto bench_hot_path 3000`
Expected: fresh source-of-record profiling artifacts

- [x] **Step 4: Update the evidence docs**

Expected:
- comparison docs cite the post-optimization artifacts
- `iohttp` and `ioguard` fit conclusions are updated
- the roadmap marks Sprint 11A as complete only after the evidence refresh

- [x] **Step 5: Run the final scan for the sprint**

Run: `podman run --rm -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest python3 scripts/lint-docs.py`
Expected: `PASS`

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS: 14`, `FAIL: 0`, `SKIP: 0`

## Execution Notes

- This sprint is release-blocking.
- `io_uring` may not remain backlog-only after this sprint; it must receive an evidence-backed accept or reject decision for the RC file path.
- SIMD may not remain generic future work after this sprint; each candidate hot function must receive an evidence-backed accept or reject decision.
- The final publication sprint must not begin until this sprint has refreshed the benchmark, profiler, and comparison pack.
