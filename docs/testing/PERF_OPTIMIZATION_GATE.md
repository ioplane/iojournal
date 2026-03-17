# Performance Optimization Gate For Sprint 11A

[![Benchmark Methodology](https://img.shields.io/badge/Benchmark-Methodology-4c8bf5)](/opt/projects/repositories/iojournal/docs/testing/BENCHMARK_METHODOLOGY.md)
[![Profiler Workflow](https://img.shields.io/badge/Profiler-Workflow-4c8bf5)](/opt/projects/repositories/iojournal/docs/testing/PROFILER_WORKFLOW.md)
[![Mermaid](https://img.shields.io/badge/Mermaid-requirementDiagram-blue)](https://mermaid.js.org/syntax/requirementDiagram.html)

## Scope

- Freeze the release-blocking optimization gate for Sprint 11A.
- Prevent unmeasured optimization changes from landing in `v0.1.0-rc.1`.
- Bind every optimization claim to one benchmark row and one profiler artifact.

```mermaid
requirementDiagram
    requirement perf_before_after {
        id: PERF-1
        text: Every optimization must record before and after measurements
        risk: high
        verifymethod: analysis
    }

    requirement perf_profiler_anchor {
        id: PERF-2
        text: Every optimization must cite one profiler artifact that explains the change
        risk: high
        verifymethod: analysis
    }

    requirement perf_scalar_valid {
        id: PERF-3
        text: Every optimization must preserve the scalar fallback path
        risk: high
        verifymethod: test
    }

    requirement perf_shared_set {
        id: PERF-4
        text: Shared optimization scenarios must stay frozen during Sprint 11A
        risk: high
        verifymethod: analysis
    }
```

## Gate Rules

| Rule | Requirement |
| --- | --- |
| Measurement | No optimization lands without a documented before and after measurement pair. |
| Profiler evidence | Every optimization cites at least one `callgrind`, `uftrace`, or equivalent profiler artifact that explains the targeted hotspot. |
| Scenario anchoring | Every optimization names one primary benchmark row that it is intended to improve. |
| Scalar fallback | Every optimization keeps the scalar baseline compiled, selectable, and testable. |
| Contract safety | Optimizations may not weaken redaction, field-shaping, or sink contract semantics to gain speed. |
| Release evidence | Optimization conclusions must be reflected in `docs/plans/comparison/PERFORMANCE_RESULTS.md` or the later hotspot inventory. |

## Frozen Shared Scenario Set

Sprint 11A optimization work uses this frozen shared scenario set:

| Scenario ID | Binary | Role |
| --- | --- | --- |
| `disabled_level` | `bench_hot_path` | short-circuit floor for non-emitting paths |
| `enabled_console` | `bench_hot_path` | synchronous encode and sink baseline |
| `medium_message` | `bench_hot_path` | medium payload without metadata-like attributes |
| `medium_message_with_metadata` | `bench_hot_path` | medium payload with metadata-like attributes |
| `contention_mpsc` | `bench_hot_path` | bounded queue and concurrent producer pressure |
| `append_file` | `bench_file_sink` | canonical shared file-append path |

Rules:

- No new shared scenario may be introduced mid-sprint without updating this document and `BENCHMARK_METHODOLOGY.md`.
- Capability-specific scenarios remain valid for appendix evidence, but they do not replace the frozen shared set.
- `append_file` remains the canonical file scenario even when the current `iojournal` artifact row name stays `append_ndjson`.

## Required Evidence Per Optimization

An optimization change is admissible only when all of the following exist:

- one before-run artifact directory under `docs/tmp/benchmarks/` or `docs/tmp/profiling/`
- one after-run artifact directory under `docs/tmp/benchmarks/` or `docs/tmp/profiling/`
- one benchmark row named explicitly in the optimization notes
- one profiler artifact named explicitly in the optimization notes
- one statement that the scalar path still passes tests and analyzer gates

## Scratch Artifact Surface

Task 2 reserves `docs/tmp/perf-analysis/` for intermediate Sprint 11A notes and run grouping.

Rules:

- `docs/tmp/perf-analysis/` is non-authoritative scratch space.
- Stable conclusions must move into `docs/testing/` or `docs/plans/comparison/`.
- Raw benchmark and profiler artifacts remain under their existing `docs/tmp/benchmarks/` and `docs/tmp/profiling/` roots.

## Non-Goals

- No optimization approval based on profiler output alone.
- No optimization approval based on benchmark output alone.
- No ISA-specific fast path without a tested scalar fallback.
