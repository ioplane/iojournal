# Benchmark Methodology For v0.1.0-rc.1

[![hyperfine](https://img.shields.io/badge/hyperfine-benchmark-4c8bf5)](https://github.com/sharkdp/hyperfine)
[![uftrace](https://img.shields.io/badge/uftrace-tracing-4c8bf5)](https://github.com/namhyung/uftrace)
[![Callgrind](https://img.shields.io/badge/Valgrind-Callgrind-4c8bf5)](https://valgrind.org/docs/manual/cl-manual.html)
[![Mermaid](https://img.shields.io/badge/Mermaid-requirementDiagram-blue)](https://mermaid.js.org/syntax/requirementDiagram.html)

## Scope

- Define the normalized benchmark contract for Sprint 09 through Sprint 11.
- Freeze artifact layout, scenario naming, and comparison boundaries for `iojournal`.
- Prevent ad hoc benchmark claims in the `v0.1.0-rc.1` evidence pack.
- Freeze the shared optimization scenario set referenced by `PERF_OPTIMIZATION_GATE.md`.

```mermaid
requirementDiagram
    requirement bench_container {
        id: BENCH-1
        text: Execute benchmark runs only inside the repository Podman image
        risk: high
        verifymethod: test
    }

    requirement bench_shared {
        id: BENCH-2
        text: Shared comparison scenarios must stay capability-neutral
        risk: high
        verifymethod: analysis
    }

    requirement bench_artifacts {
        id: BENCH-3
        text: Raw artifacts must be preserved under docs/tmp/benchmarks
        risk: medium
        verifymethod: test
    }

    requirement bench_profile {
        id: BENCH-4
        text: Published performance claims require profiler-backed interpretation
        risk: high
        verifymethod: analysis
    }
```

## Artifact Layout

- Raw benchmark runs live under `docs/tmp/benchmarks/<RUN_ID>/`.
- A run directory must contain one TSV artifact per benchmark binary.
- Optional `hyperfine` output may be added as `hyperfine.md` in the same run directory.
- Profiler artifacts live under `docs/tmp/profiling/`.
- Sprint 10 result tables must reference raw artifact directories instead of copying raw numbers into plans.

## Normalized Fixture Contract

The benchmark harness uses a frozen RC fixture family:

- timestamp is fixed to a deterministic RFC 3339-compatible UTC value
- event name and logger name are fixed per scenario family
- hot-path and file scenarios use a medium text payload with two attributes
- redaction remains enabled for shared scenarios unless a scenario explicitly measures disabled-level short-circuit behavior
- syslog loopback uses the same payload class but omits attribute bags in the current Sprint 09 baseline

Rules:

- Do not mutate field shape between compared libraries without documenting the divergence.
- Do not introduce capability-specific metadata into shared-score scenarios.
- Keep file rotation and retention disabled for steady-state file measurements.

## Scenario Catalog

| Scenario ID | Binary | Class | Release status | Notes |
| --- | --- | --- | --- | --- |
| `disabled_level` | `bench_hot_path` | shared | active | Measures short-circuit cost when `ij_logger_log()` exits before copy and sink work. |
| `enabled_console` | `bench_hot_path` | shared | active | Measures synchronous encode plus console write path with stdout redirected to `/dev/null`. |
| `append_file` | `bench_file_sink` | shared | active | Canonical shared file-append scenario. The current `iojournal` TSV row name remains `append_ndjson` because the binary emits the NDJSON-specific artifact label. |
| `udp_loopback` | `bench_syslog_udp` | capability-specific | active | Measures RFC 5424 UDP loopback path against a local receiver thread. |
| `medium_message` | `bench_hot_path` | shared | active | Isolates a medium formatted message without metadata-like attributes. |
| `medium_message_with_metadata` | `bench_hot_path` | shared | active | Isolates a medium formatted message plus metadata-like attributes. |
| `contention_mpsc` | `bench_hot_path` | shared | active | Covers concurrent producer pressure against the current bounded queue model. |

Interpretation rules:

- Only shared scenarios may appear in the primary comparison score table.
- Capability-specific scenarios must remain in a separate appendix.
- Missing profiler parity, repeatability bounds, and a performance-oriented preset record block the final Sprint 10 evidence pack.
- Sprint 11A optimization work must stay anchored to the active shared rows listed here and in `PERF_OPTIMIZATION_GATE.md`.

## Execution Rules

- Run benchmarks from repository scripts, not from ad hoc shell history.
- Use the repository Podman image for all benchmark and profiler execution.
- Capture the build preset, iteration counts, and artifact directory for every run.
- Reuse the same iteration counts across compared libraries unless a documented safety limit forces a lower bound.
- Treat `clang-debug` as harness bring-up evidence only; release-facing comparisons must also record a performance-oriented preset before Sprint 10 closes.
- Treat this document and `PERF_OPTIMIZATION_GATE.md` as the frozen shared scenario source for Sprint 11A optimization work.

Canonical entrypoint:

```bash
uv run --script scripts/benchmarks.py
```

Artifact validation requirements:

- every TSV file must start with `format\ttsv\tv1`
- every TSV file must declare the canonical columns row
- every scenario row must include `benchmark`, `scenario`, `iterations`, `elapsed_ns`, and `ns_per_op`
- latency distribution, CPU cost, and binary or dependency footprint may live outside the TSV row but remain mandatory for the Sprint 10 evidence pack
- profiler-backed hotspot explanations are mandatory companion artifacts, not optional commentary

## Comparison Rules

- Compare `iojournal` against Tier 1 libraries only on the shared scenario set.
- Do not merge capability-specific results into shared averages.
- Do not compare synchronous `iojournal` paths against asynchronous competitor modes without labeling the mismatch explicitly.
- Treat missing functionality as a functional fit result, not as a zero-filled performance row.
- Consumer-fit conclusions for `iohttp` and `ioguard` must cite both functional and performance evidence.

## Promotion Rules

- Sprint 09 closes when the harness and artifact contract exist and run cleanly.
- Sprint 10 closes only when Tier 1 measurements exist for every active shared scenario, including the normalized file-append slice.
- Sprint 11 may publish `v0.1.0-rc.1` only after profiler summaries exist for the `iojournal` hot paths behind the shared score table.

## Non-Goals

- No host-only benchmark numbers.
- No mixed shared and capability-specific score tables.
- No publication of raw numbers without traceable artifact directories.
