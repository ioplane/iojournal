# Audit Hardening and Shell Modernization Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix all critical and high-severity bugs found in the independent code audit, close the critical test coverage gaps, and rewrite all 14 shell scripts to modern Bash 5.1+ standards with full shellcheck 0.11.0 compliance.

**Architecture:** Three-phase approach: (1) fix bugs that can cause crashes or data corruption, (2) add tests for untested critical paths, (3) modernize shell scripts with structured error handling, cleanup traps, `readonly` constants, proper quoting, and shellcheck directives. No public API or ABI changes. All C fixes are internal. All shell rewrites preserve existing behavior.

**Tech Stack:** C23, CMake, Unity test framework, Bash 5.1+, shellcheck 0.11.0, Podman dev container.

---

## Sprint Decomposition

| Sprint | Theme | Primary Outcome |
|---|---|---|
| 13 | Critical bug fixes and test hardening | Memory safety, SIMD dispatch, and API contract bugs fixed; concurrency, UTF-8, SIMD equivalence, and encoder overflow tests added |
| 14 | Shell script modernization | All 14 scripts rewritten to Bash 5.1+ standards with shellcheck 0.11.0 compliance, structured error handling, cleanup traps, and consistent style |

---

## Program Map

### Sprint 13 Tasks

| Task | Scope | Files |
|---|---|---|
| 1 | Fix UTF-8 scalar OOB read | `src/core/ij_utf8_scalar.c`, test |
| 2 | Fix redaction buffer overflow guard | `src/filters/ij_redact.c`, test |
| 3 | Fix SIMD dispatch for ARM NEON | `src/encoders/ij_simd_dispatch.h`, test |
| 4 | Fix RFC 5424 APP-NAME sanitization | `src/encoders/ij_rfc5424.c`, test |
| 5 | Fix file sink clock step-back | `src/sinks/ij_file_sink.c`, test |
| 6 | Fix logger config deep copy | `src/core/ij_logger.c`, test |
| 7 | Add ring queue concurrency tests | `tests/unit/test_queue.c` |
| 8 | Add UTF-8 validation unit tests | `tests/unit/test_utf8.c` (new) |
| 9 | Add SIMD scalar-vs-AVX2 equivalence tests | `tests/unit/test_simd_dispatch.c` |
| 10 | Add encoder buffer overflow tests | `tests/unit/test_console_json.c`, `tests/unit/test_rfc5424.c` |
| 11 | Add event validation negative tests | `tests/unit/test_event.c` |

### Sprint 14 Tasks

| Task | Scope | Files |
|---|---|---|
| 1 | Create shared shell library | `scripts/lib/common.sh` (new) |
| 2 | Rewrite quality.sh | `scripts/quality.sh` |
| 3 | Rewrite release scripts | `scripts/run-release-gate.sh`, `scripts/run-release-candidate.sh`, `scripts/build-release-assets.sh`, `scripts/render-release-notes.sh` |
| 4 | Rewrite analysis scripts | `scripts/run-gcc-analyzer.sh`, `scripts/run-coverage.sh` |
| 5 | Rewrite benchmark scripts | `scripts/run-benchmarks.sh`, `scripts/run-tier1-benchmarks.sh`, `scripts/build-tier1-competitors.sh`, `scripts/fetch-tier1-competitors.sh` |
| 6 | Rewrite profiler scripts | `scripts/run-profiler-review.sh`, `scripts/run-podman-perf-lane.sh`, `scripts/build-uftrace-bench.sh` |
| 7 | Add shellcheck to quality gate | `scripts/quality.sh`, Containerfile |

---

## Exit Criteria

- `shellcheck --shell=bash --severity=style scripts/*.sh` exits 0
- `ctest --preset clang-debug` passes with all new tests
- `quality.sh` PASS: N, FAIL: 0 inside Podman
- No behavioral regression in any script (same inputs produce same outputs)
