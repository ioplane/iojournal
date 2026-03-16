# iojournal - Project Instructions for Codex

## Start Here

Read these in order before making non-trivial changes:
- `AGENTS.md`
- `CLAUDE.md`
- `docs/plans/ROADMAP.md`
- `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- `docs/plans/comparison/RAW_ARTIFACTS.md`
- `.claude/skills/ROADMAP.md`

## Core Working Rules

- Preserve the public API shape in `include/iojournal/` unless the task explicitly requires an API change.
- Keep the logging pipeline responsibilities narrow: event capture, bounded buffering, filtering, encoding, and sink delivery.
- Do not move application routing, HTTP parsing, OpenAPI validation, or unrelated transport logic into `iojournal`.
- Default to safe behavior. Any lenient or lossier behavior must be explicit, documented, and covered by tests.
- Keep the hot path allocation-free and avoid hidden unbounded work.
- Treat `docs/testing/IO_URING_RELEVANCE.md` and `docs/testing/SIMD_RELEVANCE.md` as closed RC decisions unless a new measured plan explicitly reopens them.
- Keep SIMD out of the public ABI. Do not expose vector types or ISA-specific structs in `include/iojournal/`.
- Keep ISA-specific code isolated in internal translation units or dispatch helpers so the generic path cannot accidentally execute unsupported instructions.

## Code and Review Focus

- Public names: `ij_*`
- Pipeline ownership: producer -> buffer -> filter -> encoder -> sink
- Security focus: redaction of tokens, secrets, cookies, passwords, and session identifiers
- Observability focus: RFC 5424, RFC 5425/5426/6587, NDJSON, OTLP compatibility
- Performance focus: bounded queues, backpressure policy, `io_uring`-aware sinks when applicable

## Required Follow-Through

- Update unit tests when behavior changes.
- Update docs when layer boundaries, defaults, or external contracts change.
- Keep `CLAUDE.md`, `CODEX.md`, and `.claude/skills/` aligned when repository rules change.
- Before handoff between AI agents, synchronize planning state in one change-set:
  - `docs/plans/ROADMAP.md`
  - `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
  - `docs/plans/sprints/README.md`
  - `docs/plans/sprints/<active-sprint>.md`
- Use `scripts/run-podman-perf-lane.sh` for `io_uring`, `uftrace`, `gdb`, and other ptrace-sensitive profiling work; keep the default `podman run` path for ordinary build, test, and analyzer steps.
- After every performance task, rerun `scripts/quality.sh` inside Podman before claiming progress.
- For performance work, update the active evidence docs and source-of-record artifact references before moving to the next sprint task.
- For sprint/document handoffs, mandatory checks are: `python3 scripts/lint-docs.py` and
  `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`.

## Compiler Policy

- Keep both Clang and GCC lanes healthy for C23 work. Measured and published lanes must use
  explicit `-std=c23`.
- Do not mix GCC and Clang sanitizer objects, LTO objects, or FMV resolvers in one binary or one
  evidence pack.
- For SIMD, vectorization, or hot-path loop work, collect:
  - Clang: `-Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize`
  - GCC: `-fopt-info-vec-all`
- Prefer `#pragma omp simd` for portable vectorization hints. Keep vendor-specific pragmas inside
  internal kernels with scalar fallback.
- Add `restrict` only when the non-aliasing contract is provable and test-backed.
- Keep `-ffast-math` disabled; if local FP contraction is justified, keep it narrow and documented.

## Local Skills

Repository-local skills live under `.claude/skills/`. They are the closest thing to project memory for repeated architecture, standards, and repository decisions, even when the active agent is not Claude.

Preferred combinations:
- architecture/runtime changes: `iojournal-architecture` + `iojournal-coding-standards`
- performance and profiling changes: `iojournal-performance-optimization` + `modern-c23`
- repository/docs/release changes: `iojournal-repository-conventions`

## Required Utilities

For effective Codex work on this repository, keep these available on the host:
- `git` for branches, worktrees, history, and patch-oriented review
- `gh` with working auth, especially `gh api graphql`, for repository and PR automation
- `rg` (`ripgrep`) for fast code and path discovery
- `jq` for processing JSON output from GitHub APIs, tool output, and generated reports
- `python3` for project-local automation and validation scripts
- `podman` for the development and quality execution environment
- `uv` / `uvx` for optional MCP and helper tooling such as Serena
- `clangd` for semantic C/C++ navigation when LSP-based tooling is used
- `gcovr` for GCC coverage reporting
- `scan-build` / `scan-view` for local analyzer triage
- `clang-check` and `diagtool` for Clang front-end and diagnostic inspection

Useful but optional:
- `fd` for fast filename discovery
- `yq` for YAML inspection
- `hyperfine` for repeatable benchmark comparisons
- `uftrace` for hot-path tracing
- `valgrind` for `callgrind` profiling
- `gdb` for low-level debugging

## Current Optimization Frontier

- `io_uring` is rejected for the RC-active file path on the current workload.
- SIMD is kept only for `x86_64 AVX2` JSON escape and UTF-8 validation in the RC-active measured surface.
- `arm64 NEON` remains the intended secondary SIMD target with scalar fallback; `AVX-512`, `SVE`, and `SVE2` stay post-RC.
- The current release-blocking state is Sprint 12 publication hardening; the latest profiler pack still points primarily to JSON shaping, event copy, escape scanning, and redaction rather than to I/O or queue mechanics.
