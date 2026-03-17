# Repository Guidelines

## Project Structure & Module Organization
`src/` is intended to host the logging implementation split by responsibility: `core` (ring buffers, threads, context), `encoders` (JSON, RFC5424, OTLP payload shaping), `sinks` (console, file, syslog, OTLP, HEC, Elastic), and `filters` (level, sampling, redact). Public headers belong in `include/iojournal/`; internal-only declarations stay in `src/ij_internal.h`. Unit tests belong in `tests/unit/`, integration tests and examples in `examples/`, and development tooling in `scripts/` plus `deploy/podman/`. Long-form plans belong in `docs/plans/`; `docs/tmp/` is scratch space and must not be treated as authoritative.

Bootstrap note: parts of the implementation tree may still be missing while repository infrastructure is being assembled. Treat this document as the intended steady-state layout.

## Build, Test, and Development Commands
Develop inside the container defined in `deploy/podman/Containerfile`:
```bash
podman build -t iojournal-dev:latest -f deploy/podman/Containerfile .
podman run --rm -it -v $(pwd):/workspace:Z iojournal-dev:latest
```

For `io_uring`, `uftrace`, `gdb`, and other ptrace-sensitive performance lanes, use the
repository wrapper instead of the default container launch:
```bash
bash scripts/run-podman-perf-lane.sh bash
bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh auto 3000
```

Primary local workflow once the CMake project is present:
```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```

Use `cmake --build --preset clang-debug --target format` to rewrite formatting and `--target format-check` to verify it. Run `./scripts/quality.sh` before submitting changes. During bootstrap stages, the quality script may skip build-specific steps until `CMakeLists.txt` and presets exist.

Agent execution rules:
- ordinary build, test, analyzer, and docs work stays on the default `podman run` lane
- `io_uring`, `uftrace`, `gdb`, and other ptrace-sensitive profiling work must use `scripts/run-podman-perf-lane.sh`
- after each performance or behavior-changing task, rerun the full containerized `scripts/quality.sh` gate
- performance work must update the active evidence docs under `docs/plans/comparison/` and `docs/testing/`

## Profiling Toolchain
The development image is expected to carry profiling/debug tools for log hot-path work:
- `gdb`
- `valgrind`
- `uftrace`
- `hyperfine`
- `gcovr`
- `scan-build`
- `scan-view`
- `run-clang-tidy.py`
- `clang-tidy-diff.py`
- `diagtool`
- `clang-check`
- `ftracer` helpers (`frun`, `fresolve`, `/usr/local/lib/ftracer/ftracer.o`)
- GCC 15 analysis helpers: `gcov`, `gcov-tool`, `gcov-dump`, `lto-dump`

Use them after a normal debug build. Typical sequence:
```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
```
Then choose the tool that matches the question:
```bash
bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh uftrace bench_hot_path 2000 medium_message_with_metadata
bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh callgrind bench_file_sink 3000 append_ndjson
bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh gdb bench_hot_path 1000 enabled_console
```

`io_uring` experiments and ptrace-sensitive profiling must run through
`scripts/run-podman-perf-lane.sh`. The default `podman run` path stays authoritative for normal
build, test, and analyzer work.

## Compiler And Vectorization Policy
- Keep both Clang and GCC lanes active for C23 work. Published and measured lanes must pass with
  an explicit `-std=c23`; do not treat GCC's default `gnu23` mode as the repository contract.
- Do not mix GCC and Clang sanitizer objects, LTO objects, or function-multi-versioned resolvers
  in one binary, one benchmark artifact, or one release verification pack.
- For hot-path, SIMD, or vectorization-sensitive changes, capture feedback from both compilers:
  - Clang: `-Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize`
  - GCC: `-fopt-info-vec-all`
- Prefer `#pragma omp simd` for portable vectorization hints. Use compiler-specific pragmas only in
  internal kernels, with scalar fallback and measured justification.
- Add `restrict` only when the non-aliasing contract is provable from the API or internal
  ownership rules and is covered by tests or invariants.
- Keep `-ffast-math` disabled. If a measured hot path needs FP contraction or other floating-point
  tuning, scope it locally and document the reason instead of widening the repository baseline.
- When reopening `arm64` SIMD work, keep a Clang lane in scope because `NEON` and future SVE-family
  studies are not expected to track GCC and Clang codegen quality equally.

Current release-blocking performance evidence:
- local benchmark source of record: `docs/tmp/benchmarks/20260316-160705/`
- Tier 1 comparison source of record: `docs/tmp/benchmarks/20260316-150320/`
- callgrind and repeatability source of record: `docs/tmp/profiling/20260316-164059/`
- `uftrace` companion runs: `docs/tmp/profiling/20260316-164602/`, `docs/tmp/profiling/20260316-164658/`, `docs/tmp/profiling/20260316-164607/`
- `io_uring` decision: `docs/testing/IO_URING_RELEVANCE.md`
- SIMD decision: `docs/testing/SIMD_RELEVANCE.md`
- current profiler conclusion: the hottest remaining RC rows are still dominated by `ij_json_console_encode`, `ij_event_copy_from_input`, JSON escape scanning, and redaction classification; Sprint 12 now owns the publish/no-publish decision.
- SIMD policy:
  - keep the public ABI scalar-only; do not expose vector types in `include/iojournal/`
  - keep ISA-specific code in internal translation units and internal dispatch headers only
  - keep runtime feature detection cached and isolated from the generic path
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

## Documentation Style
Write English documentation first. Treat `docs/en/*` as authoritative and `docs/ru/*` as the translation/adaptation layer once the stable docs surface exists.

Documentation must use strict technical language:
- write facts, contracts, limits, ownership rules, and examples
- do not write narrative introductions, philosophy sections, or marketing text
- keep comparisons measurable and testable
- keep non-goals explicit and tied to API or layer boundaries
- use Mermaid for diagrams
- use extended GitHub Markdown only where it improves technical readability:
  tables, fenced code blocks with language tags, short blockquote alerts for strict warnings, and `<details>` for secondary examples
- use badges in every stable document; badge links must point to official sources, official repositories, or official project sites
- Russian documentation must use Russian prose; keep English only for API identifiers, macro names, external project names, protocol names, and RFC identifiers

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
- when planning or executing release-blocking performance work, treat `docs/plans/ROADMAP.md`, `docs/plans/comparison/PERFORMANCE_RESULTS.md`, `docs/plans/comparison/RAW_ARTIFACTS.md`, and `docs/testing/*RELEVANCE.md` as authoritative before `docs/tmp/`

## Changelog And Versioning
Maintain [CHANGELOG.md](CHANGELOG.md) for every merge that changes behavior, public API, verification evidence, release automation, or published documentation.

Rules:
- Follow [Keep a Changelog 1.1.0](https://keepachangelog.com/en/1.1.0/).
- Keep `Unreleased` at the top.
- Add entries only under the standard sections that apply:
  `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, `Security`.
- Write short factual bullets. Do not write narrative paragraphs in the changelog.
- Record externally visible changes only. Do not list routine refactors unless they change behavior, contract, or release evidence.
- Update the changelog in the same branch as the code or documentation change. Do not defer it to a later cleanup branch.

Versioning rules:
- Use Semantic Versioning tags with a leading `v`.
- Current pre-1.0 policy:
  - `v0.y.z` for normal unstable releases
  - `v0.y.z-rc.N` for release candidates
- Do not create `1.0.0` until public API, release gate, and published evidence are all declared stable.
- For this repository, the first public release candidate line is `v0.1.0-rc.N`.

## Coding Style & Naming Conventions
Target C23 only; C extensions are disabled. Follow `.clang-format`: 4-space indentation, Linux brace style, 100-column limit, and right-aligned pointer stars (`int *ptr`). Keep public and internal symbols consistent with the existing scheme: `ij_` for functions, `IJ_` for macros and enum values, and `_t` for typedefs. Prefer small, single-purpose translation units and keep the hot logging path bounded and allocation-free.

## Testing Guidelines
Unit tests use Unity and are enabled through `IOJOURNAL_BUILD_TESTS`. Name new unit tests `tests/unit/test_<area>.c` and register them in `CMakeLists.txt`. Concurrency structures such as ring buffers and queues require stress coverage for races, ABA risks, and backpressure behavior. Validate at least `ctest --preset clang-debug`; sanitizer presets such as `clang-asan` and `clang-tsan` are preferred for concurrency or memory-safety changes.

## Commit & Pull Request Guidelines
Follow the established convention: `feat:`, `fix:`, `refactor:`, `test:`, and `docs:` with short imperative subjects. Pull requests should describe the behavior change, list the commands run, and note any analyzer results or skipped checks. Include sample inputs or output snippets when modifying sink behavior, structured formats, or redaction rules.
