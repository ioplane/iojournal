# Sprint 09: Benchmark Harness And Methodology Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** create the release-blocking benchmark harness, profiler workflow, GCC/Clang tooling lanes, and methodology documents required before competitor comparison.

**Architecture:** this sprint must not add new sink features. It may add benchmark harness code, profiler scripts, methodology docs, and analysis lanes.

**Tech Stack:** C23 benchmark binaries, Podman, Clang 22, GCC 15, `hyperfine`, `uftrace`, `valgrind`, `gcovr`, `scan-build`.

---

### Task 1: Add benchmark and profiler scaffolding

**Files:**
- Create: `bench/CMakeLists.txt`
- Create: `bench/bench_hot_path.c`
- Create: `bench/bench_file_sink.c`
- Create: `bench/bench_syslog_udp.c`
- Create: `scripts/run-benchmarks.sh`
- Create: `scripts/run-profiler-review.sh`

- [x] **Step 1: Write the failing benchmark build wiring**

Run: `cmake --preset clang-debug && cmake --build --preset clang-debug --target bench_hot_path`
Expected: benchmark targets are missing before this sprint.

- [x] **Step 2: Implement benchmark harness**

Expected: runnable benchmark targets exist for the normalized RC scenarios.

- [x] **Step 3: Verify benchmark harness**

Run: `bash scripts/run-benchmarks.sh`
Expected: benchmark commands execute and raw artifacts are generated.

### Task 2: Add methodology and toolchain docs

**Files:**
- Create: `docs/testing/BENCHMARK_METHODOLOGY.md`
- Create: `docs/testing/PROFILER_WORKFLOW.md`
- Modify: `docs/en/04-tooling-and-agent-workflow.md`
- Modify: `docs/ru/04-tooling-and-agent-workflow.md`

- [x] **Step 1: Write methodology docs**

Expected: scenario normalization, artifact policy, and comparison rules are written down.

- [x] **Step 2: Freeze tooling lanes**

Expected: Clang, GCC, coverage, and profiler tools are named and justified.

- [x] **Step 3: Verify docs surface**

Run: `python3 scripts/lint-docs.py`
Expected: docs lint passes.

### Task 3: Add GCC and coverage lanes

**Files:**
- Modify: `CMakePresets.json`
- Modify: `scripts/quality.sh`
- Modify: `scripts/run-coverage.sh`
- Create: `scripts/run-gcc-analyzer.sh`

- [x] **Step 1: Write failing GCC/coverage checks**

Run: `bash scripts/run-gcc-analyzer.sh`
Expected: GCC analysis lane is incomplete before implementation.

- [x] **Step 2: Implement GCC and `gcovr` paths**

Expected: `-fanalyzer`, `gcov`, and `gcovr` have documented and runnable entry points.

- [x] **Step 3: Verify the new lanes**

## Execution Notes

- Benchmark harness now lives under `bench/` with `bench_hot_path`, `bench_file_sink`, and
  `bench_syslog_udp`.
- Raw benchmark artifacts are emitted by `scripts/run-benchmarks.sh` under `docs/tmp/benchmarks/`.
- Profiler entrypoints are frozen in `scripts/run-profiler-review.sh`.
- GCC analyzer evidence is emitted by `scripts/run-gcc-analyzer.sh` under `build/gcc-analyzer/analyzer/`.
- Coverage evidence now includes both Clang source-based coverage and GCC `gcovr` output through
  `scripts/run-coverage.sh`.

Run: `bash scripts/quality.sh && bash scripts/run-coverage.sh`
Expected: Clang and GCC evidence paths both execute successfully.
