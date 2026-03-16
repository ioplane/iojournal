# Sprint 11C: Structural Hot-Path Redesign Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** reduce the remaining enabled-path CPU cost in `iojournal` without changing the public API,
sink surface, or RC behavior.

**Architecture:** keep the synchronous RC contract and rework only internal hot-path structure. This
sprint fuses event validation and copy work, makes the encoder length-aware, reduces scalar
formatting overhead, and tightens redaction-key dispatch before refreshing the full evidence pack.

**Tech Stack:** Podman, CMake presets, Unity, `hyperfine`, `uftrace`, `valgrind --tool=callgrind`,
`PVS-Studio`, `CodeChecker`, `cppcheck`, C23.

---

## Task 1: Fuse validation and event copy passes

**Files:**
- Modify: `src/core/ij_event.c`
- Modify: `src/ij_internal.h`
- Modify: `tests/unit/test_event.c`

- [x] **Step 1: Add failure-first coverage**

Expected coverage:
- copied events keep the current rejection rules
- copied events keep the current owned-string layout and redaction semantics
- duplicate-key and invalid UTF-8 failures still surface exactly as before

- [x] **Step 2: Replace repeated event passes with a fused path**

Expected implementation:
- avoid repeated `strlen` and repeated validation traversals where the same scan can produce both
  validity and copy metadata
- keep the same observable success and failure behavior

- [x] **Step 3: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS`

## Task 2: Make the JSON and NDJSON builders length-aware

**Files:**
- Modify: `src/encoders/ij_json_console.c`
- Modify: `src/encoders/ij_ndjson.c`
- Modify: `tests/unit/test_console_json.c`
- Modify: `tests/unit/test_file_sink.c`

- [x] **Step 1: Add failure-first coverage for length-aware encoding**

Expected coverage:
- unchanged JSON output
- unchanged NDJSON output
- unchanged escaping and attribute formatting behavior

- [x] **Step 2: Replace `strlen`-driven builder calls with length-aware calls**

Expected implementation:
- reuse `ij_owned_string_t.len`
- keep scalar and SIMD escape behavior unchanged
- remove redundant string-length scans from the encode path

- [x] **Step 3: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS`

## Task 3: Reduce timestamp and scalar formatting overhead

**Files:**
- Modify: `src/encoders/ij_json_console.c`
- Modify: `tests/unit/test_console_json.c`
- Modify: `tests/unit/test_rfc5424.c`

- [x] **Step 1: Add failure-first coverage**

Expected coverage:
- unchanged RFC3339 timestamp text
- unchanged numeric formatting for signed, unsigned, boolean, and double values

- [x] **Step 2: Replace generic formatting where fixed-format append is sufficient**

Expected implementation:
- reduce hot-path `snprintf` usage where output width and structure are already fixed
- preserve exact emitted text

- [x] **Step 3: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS`

## Task 4: Rewrite redaction key dispatch

**Files:**
- Modify: `src/filters/ij_redact.c`
- Modify: `tests/unit/test_filters.c`

- [x] **Step 1: Add failure-first coverage**

Expected coverage:
- exact same redaction keys
- exact same suffix behavior for dotted keys
- exact same non-match behavior

- [x] **Step 2: Replace linear matching with a cheaper dispatch structure**

Expected implementation:
- length-aware and first-byte-aware dispatch
- no change to the key list or redact decision semantics

- [x] **Step 3: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS`

## Task 5: Refresh benchmark and profiler evidence

**Files:**
- Modify: `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- Modify: `docs/plans/comparison/RAW_ARTIFACTS.md`
- Modify: `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
- Modify: `docs/plans/sprints/README.md`
- Modify: `CHANGELOG.md`

- [x] **Step 1: Re-run the local benchmark baseline**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash -lc 'PRESET=clang-perf bash scripts/run-benchmarks.sh'`
Expected: refreshed source-of-record benchmark artifacts.
Result: quiet local source-of-record benchmark run [`20260316-184502-quiet`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-184502-quiet).

- [x] **Step 2: Re-run the profiler pack**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash -lc 'PRESET=clang-perf bash scripts/run-profiler-review.sh auto 3000'`
Run: `bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh uftrace bench_hot_path 2000 medium_message_with_metadata`
Run: `bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh uftrace bench_hot_path 2000 contention_mpsc`
Run: `bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh uftrace bench_file_sink 2000 append_ndjson`
Expected: refreshed callgrind, `hyperfine`, and `uftrace` artifacts for the structural redesign.
Result: callgrind and repeatability pack [`20260316-184513`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-184513) with `uftrace` companions [`20260316-184711`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-184711), [`20260316-184737`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-184737), and [`20260316-184848`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-184848).

- [x] **Step 3: Update the evidence docs**

Expected:
- new benchmark and profiler source-of-record IDs are promoted
- comparison docs explain the remaining gap or the achieved lead per row
- publication decision docs are updated only after the refreshed evidence exists

## Execution Notes

- This sprint is release-blocking.
- Do not change the public API, sink families, or synchronous contract.
- Do not reopen async worker semantics, `io_uring`, or new SIMD scope here.
- After each task, the full containerized quality scan is mandatory.
