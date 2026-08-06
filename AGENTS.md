# iojournal — Agent Contract

Canonical instructions for AI coding agents and contributors working on this
repository. Claude Code, Codex, and Gemini CLI all read this file.

## Quick Facts

- **Language**: C23 (`-std=c23`, `CMAKE_C_EXTENSIONS OFF`; C extensions disabled)
- **Project**: structured logging library for the `io*` ecosystem
- **License**: intended to remain compatible with the surrounding `io*` stack
- **Platform**: Linux-first development with dedicated Podman perf lanes for `io_uring` and tracing work
- **Primary use**: shared logging foundation for `iohttp`, `ioguard`, and related `io*` components

## Project Structure & Module Organization

`src/` hosts the logging implementation split by responsibility: `core` (ring
buffers, threads, context), `encoders` (JSON, RFC5424, OTLP payload shaping),
`sinks` (console, file, syslog, OTLP, HEC, Elastic), and `filters` (level,
sampling, redact). Public headers belong in `include/iojournal/`; internal-only
declarations stay in `src/ij_internal.h`. Unit tests belong in `tests/unit/`,
integration tests and examples in `examples/`, and development tooling in
`scripts/` plus `deploy/podman/`. Long-form plans belong in `docs/plans/`;
`docs/tmp/` is scratch space and must not be treated as authoritative.

Bootstrap note: parts of the implementation tree may still be missing while
repository infrastructure is being assembled. Treat this document as the
intended steady-state layout.

Key directories:

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

## Build, Test, and Development Commands

Primary local workflow once the CMake project is present:

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```

Use `cmake --build --preset clang-debug --target format` to rewrite formatting
and `--target format-check` to verify it.

Run `python scripts/quality.py` before submitting changes. During bootstrap stages,
the quality script may skip build-specific steps until `CMakeLists.txt` and
presets exist; use `python scripts/quality.py` for the repository-level checks that
can run before the CMake surface exists.

Agent execution rules:

- ordinary build, test, analyzer, and docs work stays on the default `podman run` lane
- `io_uring`, `uftrace`, `gdb`, and other ptrace-sensitive profiling work must use `scripts/podman_perf_lane.py`
- after each performance or behavior-changing task, rerun the full containerized `python scripts/quality.py` gate
- performance work must update the active evidence docs under `docs/plans/comparison/` and `docs/testing/`

For AI handoff and plan-gate edits, make planning edits and evidence updates
consistent in one change-set:

- `docs/plans/ROADMAP.md`
- `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
- `docs/plans/sprints/README.md`
- active sprint file under `docs/plans/sprints/`

Mandatory handoff checks, in sequence:

```bash
python3 scripts/lint-docs.py
uv run --script scripts/release_candidate.py
uv run --script scripts/release_assets.py v0.1.0-rc.1
uv run --script scripts/release_notes.py v0.1.0-rc.1
podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env \
  -v $(pwd):/workspace:Z -w /workspace \
  localhost/iojournal-dev:latest python scripts/quality.py
```

## Dev Container

Prefer doing development and quality checks inside the container.

```bash
# Shared toolchain base layer (all io* projects use localhost/ioplane-base:latest)
podman build -t ioplane-base:latest \
  -f /opt/projects/repositories/container-images/tools/ioplane-base/Containerfile .

# Project image
podman build -t iojournal-dev:latest -f deploy/podman/Containerfile .

# Default run lane
podman run --rm -it -v $(pwd):/workspace:Z iojournal-dev:latest

# Perf / io_uring / ptrace-sensitive lane
uv run --script scripts/podman_perf_lane.py bash
uv run --script scripts/podman_perf_lane.py bash scripts/run-profiler-review.sh auto 3000
```

- All `io*` projects share `localhost/ioplane-base:latest` as the common toolchain layer.
- Use the dedicated perf lane for `io_uring`, `uftrace`, `gdb`, and other ptrace-sensitive runs.
- The default `podman run` path stays authoritative for normal build, test, and analyzer work.
- After each performance task, rerun the full `python scripts/quality.py` gate inside Podman.

## Profiling Toolchain

The development image carries profiling/debug tools for log hot-path work,
organized by lane:

| Lane  | Tools |
| ----- | ----- |
| Clang | `clang-tidy`, `run-clang-tidy.py`, `clang-tidy-diff.py`, `scan-build`, `scan-view`, `diagtool`, `clang-check` |
| GCC   | `-fanalyzer`, `gcov`, `gcovr`, `gcov-tool`, `gcov-dump`, `lto-dump` |
| Perf  | `hyperfine`, `uftrace`, `valgrind` (`--tool=callgrind`), `gdb`, `ftracer` helpers (`frun`, `fresolve`, `/usr/local/lib/ftracer/ftracer.o`) |

Use them after a normal debug build. Typical sequence:

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
```

Then choose the tool that matches the question:

```bash
uv run --script scripts/podman_perf_lane.py bash scripts/run-profiler-review.sh uftrace bench_hot_path 2000 medium_message_with_metadata
uv run --script scripts/podman_perf_lane.py bash scripts/run-profiler-review.sh callgrind bench_file_sink 3000 append_ndjson
uv run --script scripts/podman_perf_lane.py bash scripts/run-profiler-review.sh gdb bench_hot_path 1000 enabled_console
```

`io_uring` experiments and ptrace-sensitive profiling must run through
`scripts/podman_perf_lane.py`. The default `podman run` path stays
authoritative for normal build, test, and analyzer work.

## Compiler And Vectorization Policy

- Keep both Clang and GCC lanes active and healthy for C23 work. Published and
  measured lanes must pass with an explicit `-std=c23`; do not treat GCC's
  default `gnu23` mode as the repository contract.
- Do not mix GCC and Clang sanitizer objects, LTO objects, or
  function-multi-versioned (FMV) resolvers in one binary, one benchmark
  artifact, or one release verification pack.
- For hot-path, SIMD, or vectorization-sensitive changes, capture feedback from both compilers:
  - Clang: `-Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize`
  - GCC: `-fopt-info-vec-all`
- Prefer `#pragma omp simd` for portable vectorization hints. Use
  compiler-specific pragmas only in internal kernels, paired with scalar
  fallback and measured justification.
- Add `restrict` only when the non-aliasing contract is provable from the API or
  internal ownership rules and is covered by tests or invariants.
- Keep `-ffast-math` disabled. If a measured hot path needs FP contraction or
  other floating-point tuning, scope it locally and document the reason instead
  of widening the repository baseline.
- When reopening `arm64` SIMD work, keep a Clang lane in scope because `NEON` and
  future SVE-family studies are not expected to track GCC and Clang codegen
  quality equally.

## Current Release-Blocking Evidence

Two evidence snapshots are on record from separate profiling passes. Keep both
until they are reconciled; treat the listed docs under `docs/plans/comparison/`
and `docs/testing/` as authoritative before anything in `docs/tmp/`.

Snapshot A:

- local benchmark source of record: `docs/tmp/benchmarks/20260316-210134/`
- Tier 1 benchmark source of record: `docs/tmp/benchmarks/20260316-150320/`
- callgrind and repeatability source of record: `docs/tmp/profiling/20260316-210204/`
- `uftrace` companion sources: `docs/tmp/profiling/20260316-210329/`, `docs/tmp/profiling/20260316-210345/`, `docs/tmp/profiling/20260316-210349/`
- profiler conclusion: remaining RC cost is still concentrated in JSON shaping, event copy, escape scanning, and redaction; Sprint 12 now owns the publication decision.

Snapshot B:

- local benchmark source of record: `docs/tmp/benchmarks/20260316-160705/`
- Tier 1 comparison source of record: `docs/tmp/benchmarks/20260316-150320/`
- callgrind and repeatability source of record: `docs/tmp/profiling/20260316-164059/`
- `uftrace` companion runs: `docs/tmp/profiling/20260316-164602/`, `docs/tmp/profiling/20260316-164658/`, `docs/tmp/profiling/20260316-164607/`
- profiler conclusion: the hottest remaining RC rows are still dominated by `ij_json_console_encode`, `ij_event_copy_from_input`, JSON escape scanning, and redaction classification; Sprint 12 now owns the publish/no-publish decision.

Shared decisions and policy:

- `io_uring` decision: `docs/testing/IO_URING_RELEVANCE.md`
- SIMD decision: `docs/testing/SIMD_RELEVANCE.md`
- SIMD policy:
  - keep the public ABI scalar-only; do not expose vector types in `include/iojournal/`
  - isolate ISA-specific code in internal translation units and internal dispatch headers only
  - keep runtime feature detection cached, explicit, and isolated from the generic hot path
  - RC-active SIMD scope remains `x86_64 AVX2` plus `arm64 NEON` with scalar fallback
  - `AVX-512`, `SVE`, and `SVE2` are post-RC only unless a new measured plan reopens them

## Required Host Utilities

Keep these available on the host for effective agent and contributor workflows:

- `git`
- `gh` with working `gh api graphql` authentication
- `rg` (`ripgrep`)
- `jq`
- `python3`
- `podman`

Strongly recommended:

- `uv` / `uvx`
- `clangd`
- `fd`
- `yq`
- `hyperfine`
- `gdb`
- `valgrind`
- `uftrace`
- `ftracer`
- `gcovr`
- `scan-build`
- `scan-view`
- `clang-check`
- `diagtool`

## Architecture Rules

- Keep the pipeline split explicit: producer -> buffer -> filter -> encoder -> sink.
- Keep the hot logging path bounded and free of dynamic allocation.
- Keep transport, exporter, and sink behavior separate from event capture and filtering.
- Default to safe redaction and conservative backpressure behavior.
- Treat `iohttp`, `liboas`, and other consumers as integration profiles, not as reasons to blur library boundaries.
- Keep SIMD as an internal implementation detail, not as part of the public ABI contract.

## Documentation Style

Write English documentation first. Treat `docs/en/*` as authoritative and
`docs/ru/*` as the translation/adaptation layer once the stable docs surface
exists.

Documentation must use strict technical language:

- write facts, contracts, limits, ownership rules, and examples
- do not write narrative introductions, philosophy sections, or marketing text
- keep comparisons measurable and testable
- keep non-goals explicit and tied to API or layer boundaries
- use Mermaid for diagrams
- use extended GitHub Markdown only where it improves technical readability:
  tables, fenced code blocks with language tags, short blockquote alerts for
  strict warnings, and `<details>` for secondary examples
- use badges in every stable document; badge links must point to official
  sources, official repositories, or official project sites
- Russian documentation must use Russian prose; keep English only for API
  identifiers, macro names, external project names, protocol names, and RFC
  identifiers

Mermaid diagram type must match the content:

- architecture: `architecture-beta` or `flowchart`
- interaction or handoff: `sequenceDiagram`
- state transitions: `stateDiagram-v2`
- release gates or requirements: `requirementDiagram`
- classification or topology: `mindmap`, `block`, or `flowchart`
- comparison positioning: `quadrantChart` only when axes are explicit and measurable
- prefer the official Mermaid syntax page for the selected diagram type when linking Mermaid badges

Keep the stable structure aligned with other `io*` projects:

- `docs/README.md`
- `docs/en/README.md`
- `docs/ru/README.md`
- numbered documents in `docs/en/` and `docs/ru/`
- `docs/plans/README.md`, `docs/plans/ROADMAP.md`, `docs/plans/BACKLOG.md`

`docs/tmp/` is non-authoritative scratch space.

For agent-maintained repository memory:

- use `.claude/skills/` as the compact, reusable instruction surface
- keep `AGENTS.md`, `CLAUDE.md`, `CODEX.md`, and the local skills aligned when workflow or evidence rules change
- when planning or executing release-blocking performance work, treat
  `docs/plans/ROADMAP.md`, `docs/plans/comparison/PERFORMANCE_RESULTS.md`,
  `docs/plans/comparison/RAW_ARTIFACTS.md`, and `docs/testing/*RELEVANCE.md` as
  authoritative before `docs/tmp/`

## Changelog And Versioning

Maintain [CHANGELOG.md](CHANGELOG.md) for every merge that changes behavior,
public API, verification evidence, release automation, or published
documentation.

Rules:

- Follow [Keep a Changelog 1.1.0](https://keepachangelog.com/en/1.1.0/).
- Keep `Unreleased` at the top.
- Add entries only under the standard sections that apply:
  `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, `Security`.
- Write short factual bullets. Do not write narrative paragraphs in the changelog.
- Record externally visible changes only. Do not list routine refactors unless
  they change behavior, contract, or release evidence.
- Update the changelog in the same branch as the code or documentation change.
  Do not defer it to a later cleanup branch.

Versioning rules:

- Use Semantic Versioning tags with a leading `v`.
- Current pre-1.0 policy:
  - `v0.y.z` for normal unstable releases
  - `v0.y.z-rc.N` for release candidates
- Do not create `1.0.0` until public API, release gate, and published evidence are all declared stable.
- For this repository, the first public release candidate line is `v0.1.0-rc.N`.

## Coding Style & Naming Conventions

Target C23 only; C extensions are disabled. Follow `.clang-format`: 4-space
indentation, Linux brace style, 100-column limit, and right-aligned pointer
stars (`int *ptr`). Keep public and internal symbols consistent with the
existing scheme: `ij_` for functions, `IJ_` for macros and enum values, and `_t`
for typedefs. Prefer small, single-purpose translation units and keep the hot
logging path bounded and allocation-free.

- Use `nullptr`, `[[nodiscard]]`, `_Static_assert`, and checked arithmetic where size math matters.
- Match the existing public API style in `include/iojournal/` when that surface lands.

## Testing Guidelines

Unit tests use Unity and are enabled through `IOJOURNAL_BUILD_TESTS`. Name new
unit tests `tests/unit/test_<area>.c` and register them in `CMakeLists.txt`.
Validate at least `ctest --preset clang-debug`; sanitizer presets such as
`clang-asan` and `clang-tsan` are preferred for concurrency or memory-safety
changes.

- Add or update unit tests for every behavior change once the test harness exists.
- Prefer explicit tests for redaction, overflow policy, timestamp formatting, and sink/encoder boundaries.
- Concurrency structures such as ring buffers and queues require stress coverage
  for races, ABA risks, and backpressure behavior. Stress-test queue ownership,
  ring capacity logic, and concurrency-sensitive code.
- Benchmark only after scalar correctness and safety checks are stable.

## Commit & Pull Request Guidelines

Follow the established convention: `feat:`, `fix:`, `refactor:`, `test:`, and
`docs:` with short imperative subjects. Pull requests should describe the
behavior change, list the commands run, and note any analyzer results or skipped
checks. Include sample inputs or output snippets when modifying sink behavior,
structured formats, or redaction rules.

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
