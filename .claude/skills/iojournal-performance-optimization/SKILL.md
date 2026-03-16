---
name: iojournal-performance-optimization
description: Use when profiling, benchmarking, or optimizing hot-path code in iojournal. Mandatory for release-blocking performance work, SIMD changes, io_uring experiments, event-copy optimization, and benchmark evidence refresh.
---

# Iojournal Performance Optimization

## Overview

Use this skill to keep performance work measured, bounded, and aligned with the active release evidence. Do not treat optimization as free-form tuning.

Read these before touching performance-sensitive code:
- `docs/plans/ROADMAP.md`
- `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- `docs/plans/comparison/RAW_ARTIFACTS.md`
- `docs/testing/PERF_HOTSPOT_INVENTORY.md`
- `docs/testing/IO_URING_RELEVANCE.md`
- `docs/testing/SIMD_RELEVANCE.md`

## Current RC Decisions

- `io_uring` is rejected for the RC-active file path.
- SIMD is RC-active only for `x86_64 AVX2` JSON escape and UTF-8 validation.
- Keep the public ABI scalar-only; do not expose vector types in `include/iojournal/`.
- Keep ISA-specific code isolated in internal translation units and dispatch helpers.
- Keep runtime feature detection cached and off the generic hot path.
- `arm64 NEON` is the intended secondary SIMD target; `AVX-512`, `SVE`, and `SVE2` are post-RC only.
- The current release-facing profiler conclusion is that the hottest remaining rows are still dominated by JSON shaping, event copy, escape scanning, and redaction rather than by I/O or queue mechanics.
- Keep both Clang and GCC lanes in scope for compiler-sensitive optimization work; use explicit
  `-std=c23` in measured and published lanes.
- Do not mix GCC and Clang sanitizer objects, LTO objects, or FMV resolvers in one benchmark or
  release evidence pack.

## Execution Rules

- Use the default `podman run` lane for build, tests, analyzers, and docs.
- Use `scripts/run-podman-perf-lane.sh` only for `io_uring`, `uftrace`, `gdb`, and other ptrace-sensitive runs.
- Keep scalar and synchronous fallbacks correct before keeping optimized paths.
- Keep the public API and ABI free of SIMD vector types even when internal kernels are ISA-specific.
- Do not widen protocol scope while chasing performance.
- Prefer `#pragma omp simd` for portable vectorization hints; keep compiler-specific pragmas
  internal and paired with scalar fallback.
- Add `restrict` only when the non-aliasing contract is explicit, reviewable, and test-backed.
- Keep `-ffast-math` disabled; if a measured kernel needs local FP contraction, scope it narrowly
  and record the reason in the evidence docs.

## Evidence Rules

1. Every optimization must cite one benchmark row and one profiler artifact.
2. Update `docs/plans/comparison/PERFORMANCE_RESULTS.md` and `docs/plans/comparison/RAW_ARTIFACTS.md` when the source-of-record artifacts change.
3. Re-run the full containerized `scripts/quality.sh` gate after each task.
4. Treat `docs/tmp/` as raw evidence only; authoritative conclusions belong in `docs/plans/comparison/` and `docs/testing/`.
5. For loop, SIMD, or dispatch-sensitive work, archive compiler feedback from both lanes:
   - Clang: `-Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize`
   - GCC: `-fopt-info-vec-all`

## Workflow

1. Confirm the current hotspot from the active evidence docs.
2. Add or update tests before changing behavior-sensitive code.
3. Implement the narrowest profitable optimization.
4. Refresh benchmark and profiler evidence.
5. Re-run the full Podman quality gate.
6. Update plans, skills, and root instructions if the accepted workflow changed.

## References

- `references/perf-workflow.md`
