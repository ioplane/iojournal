# io_uring Relevance Decision For Sprint 11A

[![io_uring](https://img.shields.io/badge/Linux-io__uring-4c8bf5)](https://github.com/axboe/liburing)
[![Benchmark Methodology](https://img.shields.io/badge/Benchmark-Methodology-4c8bf5)](/opt/projects/repositories/iojournal/docs/testing/BENCHMARK_METHODOLOGY.md)
[![Profiler Workflow](https://img.shields.io/badge/Profiler-Workflow-4c8bf5)](/opt/projects/repositories/iojournal/docs/testing/PROFILER_WORKFLOW.md)
[![Mermaid](https://img.shields.io/badge/Mermaid-requirementDiagram-blue)](https://mermaid.js.org/syntax/requirementDiagram.html)

## Scope

- Record the Sprint 11A accept or reject decision for the file-sink `io_uring` path.
- Bind the decision to one fresh benchmark artifact and one existing profiler artifact.
- Keep the default RC file path synchronous unless `io_uring` proves materially better.

```mermaid
requirementDiagram
    requirement uring_env {
        id: URING-1
        text: Evaluate io_uring only in the dedicated Podman perf lane
        risk: high
        verifymethod: test
    }

    requirement uring_relevance {
        id: URING-2
        text: Keep io_uring out of the RC path when the measured file row is not materially better
        risk: high
        verifymethod: analysis
    }

    requirement uring_fallback {
        id: URING-3
        text: Preserve synchronous fallback outside the perf lane and on unsupported runtimes
        risk: high
        verifymethod: test
    }
```

## Environment Contract

| Item | Result |
| --- | --- |
| host kernel support | `io_uring` available and enabled |
| default Podman lane | `io_uring_setup()` blocked by seccomp profile |
| dedicated perf lane | `io_uring_setup()` succeeds |
| RC default behavior | synchronous file sink remains the default active backend |
| explicit `IO_URING` request outside perf lane | falls back to synchronous backend |

## Benchmark Evidence

### Source of record

- sync file row: [`20260316-143805/bench_file_sink.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-143805/bench_file_sink.tsv)
- `io_uring` file row: [`20260316-143805/bench_file_sink_uring.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-143805/bench_file_sink_uring.tsv)
- run manifest: [`20260316-143805/manifest.md`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-143805/manifest.md)

### Measured rows

| Backend | Scenario | Iterations | Elapsed ns | ns/op |
| --- | --- | --- | --- | --- |
| sync | `append_ndjson` | `50000` | `84727171` | `1694.54` |
| `io_uring` | `append_file_io_uring` | `50000` | `995297061` | `19905.94` |

Derived result:

- `io_uring` is about `11.75x` slower than the current synchronous file path on the normalized append scenario.

## Profiler Anchor

Source:
- [`callgrind-bench_file_sink-append_ndjson.summary.txt`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-140903/callgrind-bench_file_sink-append_ndjson.summary.txt)
- [`uftrace-bench_file_sink-append_ndjson.report.txt`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-141113/uftrace-bench_file_sink-append_ndjson.report.txt)

Hotspot interpretation:

- `ij_ndjson_encode`: `56.11%`
- `ij_json_console_encode`: `55.87%`
- `ij_event_copy_from_input`: `33.44%`
- `ij_redact_owned_attr_value`: `14.40%`
- `ij_key_should_redact`: `13.39%`
- `ij_file_sink_write`: only `1.55%`

Conclusion from the profiler anchor:

- the current shared file row is not syscall-bound
- it is encode-heavy, copy-heavy, and redact-heavy
- `io_uring` does not target the current dominant cost center

## Decision

- Reject `io_uring` as an RC-active file backend for `v0.1.0-rc.1`.
- Keep the synchronous file sink as the only default RC path.
- Keep the `io_uring` backend available only as an explicit experimental backend for Sprint 11A study and future post-RC reevaluation.
- Skip network-sink `io_uring` work in Sprint 11A because the file path did not prove relevant and the current profiler evidence is not syscall-bound.

## Implementation Consequences

| Area | Rule |
| --- | --- |
| default config | `IJ_FILE_BACKEND_AUTO` resolves to synchronous backend |
| explicit `IJ_FILE_BACKEND_IO_URING` request | attempt `io_uring`, then fall back to synchronous backend when unavailable |
| benchmark lane | `bench_file_sink_uring` must assert that the active backend really is `IO_URING` |
| release evidence | Sprint 11A must cite this rejection instead of treating `io_uring` as a pending RC optimization |

## Non-Goals

- No promotion of `io_uring` to the default file backend in Sprint 11A.
- No `io_uring` rollout to syslog, TLS, or OTLP paths in Sprint 11A.
- No claim that `io_uring` is generally unhelpful; the rejection is limited to the current RC file-row contract and environment.
