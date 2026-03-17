# iojournal - Project Instructions for Claude Code

## Quick Facts

- **Language**: C23 (`-std=c23`, `CMAKE_C_EXTENSIONS OFF`)
- **Project**: structured logging library for the `io*` ecosystem
- **License**: intended to remain compatible with the surrounding `io*` stack
- **Platform**: Linux-first development with dedicated Podman perf lanes for `io_uring` and tracing work
- **Primary use**: shared logging foundation for `iohttp`, `ioguard`, and related `io*` components

## Build Commands

Use these commands once the build system is present:

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
cmake --build --preset clang-debug --target format
cmake --build --preset clang-debug --target format-check
```

- For AI handoff and plan-gate edits, make planning edits and evidence updates consistent in one change-set:
  - `docs/plans/ROADMAP.md`
  - `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
  - `docs/plans/sprints/README.md`
  - active sprint file under `docs/plans/sprints/`
- Mandatory handoff checks in sequence: `python3 scripts/lint-docs.py`, `bash scripts/run-release-candidate.sh`, `bash scripts/build-release-assets.sh v0.1.0-rc.1`, `bash scripts/render-release-notes.sh`, and `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`.

During bootstrap stages, use `./scripts/quality.sh` for the repository-level checks that can run before the CMake surface exists.

## Dev Container

- Base image: `podman build -t ioplane-base:latest -f /opt/projects/repositories/container-images/tools/ioplane-base/Containerfile .`
- Project image: `podman build -t iojournal-dev:latest -f deploy/podman/Containerfile .`
- Run: `podman run --rm -it -v $(pwd):/workspace:Z iojournal-dev:latest`
- Perf and `io_uring` lane: `bash scripts/run-podman-perf-lane.sh bash`
- All `io*` projects share `localhost/ioplane-base:latest` as the common toolchain layer.
- Prefer doing development and quality checks inside the container.
- Use the dedicated perf lane for `io_uring`, `uftrace`, `gdb`, and other ptrace-sensitive runs.
- After each performance task, rerun the full `scripts/quality.sh` gate inside Podman.

## Analysis And Profiling Stack

- Clang lane: `clang-tidy`, `run-clang-tidy.py`, `scan-build`, `scan-view`, `diagtool`, `clang-check`
- GCC lane: `-fanalyzer`, `gcov`, `gcovr`, `gcov-tool`, `gcov-dump`, `lto-dump`
- Perf lane: `hyperfine`, `uftrace`, `valgrind --tool=callgrind`, `gdb`

## Compiler Rules

- Keep both Clang and GCC lanes healthy for C23 work; measured and published lanes use explicit
  `-std=c23` instead of relying on GCC's default `gnu23`.
- Do not mix GCC and Clang sanitizer objects, LTO objects, or FMV resolvers in one artifact.
- For vectorization-sensitive work, collect both:
  - Clang: `-Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize`
  - GCC: `-fopt-info-vec-all`
- Prefer `#pragma omp simd` for portable loop hints. Keep compiler-specific pragmas internal and
  paired with scalar fallback.
- Use `restrict` only when the aliasing contract is explicit and defensible.
- Keep `-ffast-math` disabled; scope any FP contraction locally and document it.

## Current Release-Blocking Evidence

- Local benchmark source of record: `docs/tmp/benchmarks/20260316-210134/`
- Tier 1 benchmark source of record: `docs/tmp/benchmarks/20260316-150320/`
- callgrind and repeatability source of record: `docs/tmp/profiling/20260316-210204/`
- `uftrace` companion sources:
  - `docs/tmp/profiling/20260316-210329/`
  - `docs/tmp/profiling/20260316-210345/`
  - `docs/tmp/profiling/20260316-210349/`
- `io_uring` decision: `docs/testing/IO_URING_RELEVANCE.md`
- SIMD decision: `docs/testing/SIMD_RELEVANCE.md`
- Current profiler conclusion: remaining RC cost is still concentrated in JSON shaping, event copy, escape scanning, and redaction; Sprint 12 now owns the publication decision.
- SIMD policy:
  - keep the public ABI scalar-only and keep vector types out of public headers
  - isolate ISA-specific code in internal translation units and dispatch helpers
  - keep runtime feature detection cached, explicit, and off the generic hot path
  - RC-active SIMD scope remains `x86_64 AVX2` plus `arm64 NEON` with scalar fallback
  - `AVX-512`, `SVE`, and `SVE2` are post-RC only unless a new measured plan reopens them

## Key Directories

```text
include/iojournal/       # Public API headers
src/                     # Core, encoders, sinks, filters, internal runtime
tests/unit/              # Unity tests
examples/                # Small usage examples
docs/plans/              # Architecture and implementation plans
docs/rfc/                # Local RFC mirror
docs/tmp/                # Local scratch area, not source of truth
.claude/skills/          # Repository-local skills and roadmap
```

## Architecture Rules

- Keep the pipeline split explicit: producer -> buffer -> filter -> encoder -> sink.
- Keep the hot logging path bounded and free of dynamic allocation.
- Keep transport, exporter, and sink behavior separate from event capture and filtering.
- Default to safe redaction and conservative backpressure behavior.
- Treat `iohttp`, `liboas`, and other consumers as integration profiles, not as reasons to blur library boundaries.
- Keep SIMD as an internal implementation detail, not as part of the public ABI contract.

## Code Conventions

- Public symbols use `ij_`; macros and enum values use `IJ_`; typedefs end with `_t`.
- Use `nullptr`, `[[nodiscard]]`, `_Static_assert`, and checked arithmetic where size math matters.
- Keep pointer style right-aligned: `int *ptr`.
- Match the existing public API style in `include/iojournal/` when that surface lands.

## Testing Rules

- Add or update unit tests for every behavior change once the test harness exists.
- Prefer explicit tests for redaction, overflow policy, timestamp formatting, and sink/encoder boundaries.
- Stress-test queue ownership, ring capacity logic, and concurrency-sensitive code.
- Benchmark only after scalar correctness and safety checks are stable.

## Skills Reference

Base skills live in `.claude/skills/`:
- `iojournal-architecture`
- `iojournal-coding-standards`
- `iojournal-performance-optimization`
- `logging-rfc-reference`
- `modern-c23`
- `iojournal-repository-conventions`

Roadmap: `.claude/skills/ROADMAP.md`

Agent workflow:
- architecture or runtime work: `iojournal-architecture` + `iojournal-coding-standards`
- performance work: `iojournal-performance-optimization` + `modern-c23`
- repo/docs/release work: `iojournal-repository-conventions`
