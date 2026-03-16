# Performance Workflow

## Authoritative Inputs

- `docs/plans/ROADMAP.md`
- `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- `docs/plans/comparison/RAW_ARTIFACTS.md`
- `docs/testing/PERF_HOTSPOT_INVENTORY.md`
- `docs/testing/IO_URING_RELEVANCE.md`
- `docs/testing/SIMD_RELEVANCE.md`

## Container Rules

- Default work:
  - `podman run --rm -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest ...`
- Perf lane:
  - `bash scripts/run-podman-perf-lane.sh ...`

Use the perf lane only for:
- `uftrace`
- `gdb`
- `io_uring` experiments
- other ptrace-sensitive or seccomp-sensitive work

## Required Verification

- `podman run --rm -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest python3 scripts/lint-docs.py`
- `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`

## Current Next Step

- keep the final RC publication sprint gated on the refreshed profiler evidence and on the current hotspot ordering, which still points to JSON shaping, event copy, escape scanning, and redaction

## SIMD Policy

- Keep the public ABI scalar-only; SIMD stays internal.
- Keep ISA-specific kernels isolated in internal translation units or dispatch helpers.
- Cache runtime feature detection and keep it out of per-call hot paths.
- RC-active measured SIMD scope is `x86_64 AVX2` plus `arm64 NEON` with scalar fallback.
- `AVX-512`, `SVE`, and `SVE2` are post-RC topics unless a new measured plan explicitly reopens them.
