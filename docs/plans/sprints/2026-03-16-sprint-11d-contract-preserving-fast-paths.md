# Sprint 11D: Contract-Preserving Fast Paths Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** reduce the remaining `zlog` gap on enabled shared rows without changing the public API,
ownership model, redaction semantics, or emitted text contracts.

**Architecture:** only internal fast paths are allowed. This sprint must not add async semantics,
weaken validation, widen SIMD scope, or introduce a second public behavior model.

**Tech Stack:** Podman, CMake presets, Unity, `hyperfine`, `uftrace`, `valgrind --tool=callgrind`,
`PVS-Studio`, `CodeChecker`, `cppcheck`, C23.

---

## Task 1: Specialize fixed-fragment JSON shaping

**Files:**
- Modify: `src/encoders/ij_json_console.c`
- Modify: `tests/unit/test_console_json.c`
- Modify: `tests/unit/test_file_sink.c`

- [x] **Step 1: Add failure-first exact-output coverage**

Expected coverage:
- exact output remains unchanged for fixed keys and level text
- field order remains unchanged
- NDJSON output remains exact

- [x] **Step 2: Replace constant-key builder churn with fixed fragments**

Expected implementation:
- constant keys use raw fixed fragments rather than repeated generic key builders
- level text uses a fixed quoted fragment rather than generic quoted-string assembly
- dynamic keys and dynamic string escaping remain on the existing safe path

- [x] **Step 3: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS`

## Task 2: Tighten event-copy scans without changing semantics

**Files:**
- Modify: `src/core/ij_event.c`
- Modify: `src/ij_internal.h`
- Modify: `tests/unit/test_event.c`

- [x] **Step 1: Add failure-first parity coverage**

Expected coverage:
- unchanged duplicate-key behavior
- unchanged UTF-8 rejection behavior
- unchanged aggregate-size rejection behavior
- unchanged ownership and dispose behavior

- [x] **Step 2: Reduce repeated scans and dispatch**

Expected implementation:
- remove repeated `strnlen` and repeated dispatch where the same result can be safely reused
- preserve exact success and failure semantics

- [x] **Step 3: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS`

## Task 3: Reassess redaction-path tightening

**Files:**
- Modify: `src/filters/ij_redact.c`
- Modify: `tests/unit/test_filters.c`
- Modify: `docs/plans/comparison/PERFORMANCE_RESULTS.md`

- [x] **Step 1: Add exhaustive parity coverage**

Expected coverage:
- exact-key parity
- dotted-suffix parity
- known non-match parity

- [x] **Step 2: Only implement if the profiler still justifies it**

Expected implementation:
- any additional tightening must preserve exact redaction outcomes
- if the measured gain is weak relative to risk, document the decision and skip the code change

- [x] **Step 3: Run the full scan**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: `PASS`

## Task 4: Refresh benchmark and profiler evidence

**Files:**
- Modify: `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- Modify: `docs/plans/comparison/RAW_ARTIFACTS.md`
- Modify: `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
- Modify: `docs/plans/sprints/README.md`
- Modify: `CHANGELOG.md`

- [x] **Step 1: Re-run the quiet benchmark baseline**

Expected:
- updated quiet source-of-record benchmark artifacts
- no hidden build-stage contamination in the promoted benchmark pack

- [x] **Step 2: Re-run the profiler pack**

Expected:
- refreshed callgrind, `hyperfine`, and `uftrace` artifacts for the accepted slices

- [x] **Step 3: Update the evidence docs**

Expected:
- source-of-record IDs are promoted only for accepted slices
- remaining gaps versus `zlog` are explicit
- Sprint 12 publication status is revised only after the evidence pack is complete

## Risk Rule

- Do not implement a common event-shape specialized path in this sprint unless Tasks 1 through 3
  fail to close the remaining release-relevant gap.
- Do not accept any optimization that weakens redaction or changes emitted text.

## Current Decision

- Task 1 through Task 3 code changes are accepted and remain active.
- Evidence is now promoted to the Sprint 11D active source set:
  - local benchmark source-of-record [`20260316-210134`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134)
  - profiler source-of-record [`20260316-210204`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204)
  - `uftrace` companions [`20260316-210329`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210329), [`20260316-210345`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210345), and [`20260316-210349`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210349)
- Acceptance basis:
  - enabled shared rows (`enabled_console`, `medium_message`, `medium_message_with_metadata`, `contention_mpsc`) show stable gains versus the prior 11C active source.
  - `append_ndjson` has a stronger gain versus 11C and remains primary for file-path comparisons.
  - repeatability and profiler evidence were refreshed for all three accepted rows.
- Next implementation focus:
  - Sprint 12 publication-hardening continues with 11D evidence closure.
  - keep the frontier inside the current contract-preserving constraints and only optimize copy/scan/encode work.
