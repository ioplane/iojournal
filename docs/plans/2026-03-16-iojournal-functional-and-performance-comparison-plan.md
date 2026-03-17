# iojournal Functional And Performance Comparison Plan

## Goal

Define the release-blocking comparison program required before publishing `v0.1.0-rc.1`.

The comparison program must answer two questions:

1. Is `iojournal` functionally suitable for `iohttp` and `ioguard` compared with plausible pure C alternatives?
2. Is `iojournal` performance acceptable, explainable, and evidence-backed on normalized scenarios?

## Release-Blocking Tier Set

### Tier 1: required before `v0.1.0-rc.1`

- `zlog`
- `stumpless`
- `tinylog`

### Tier 2: reference-only appendix

- `microlog`
- `rxi/log.c`
- `zf_log`
- `clog`
- `liblogax`
- `EasyLogger`

Tier 2 enriches the analysis but must not block the first RC publish decision.

## Functional Comparison Scope

The functional gate must stay aligned with `iohttp` and `ioguard` needs, not with generic logging-library tourism.

### Required dimensions

- buildability in the Podman toolchain
- integration friction and dependency burden
- runtime model: synchronous, asynchronous, thread-safe, process-safe
- file logging and operational file behavior
- syslog or equivalent network logging surface
- level filtering and disabled-level behavior
- structured or semi-structured log shaping
- operability: reload model, config friction, failure handling
- license fit and maintenance status

### Required outputs

- normalized feature matrix
- consumer-fit matrix for `iohttp` and `ioguard`
- explicit suitability conclusion per Tier 1 library

## Performance Comparison Scope

The performance gate must combine normalized measurements and profiler-backed interpretation.

### Shared scenarios

- disabled log-call overhead
- enabled synchronous file logging
- enabled console-like logging
- multi-thread contention
- medium formatted message
- medium formatted message with metadata-like context

### Capability-specific scenarios

- syslog path
- async or nonblocking path
- process-safe shared-file path

Capability-specific scenarios must be labeled clearly and must not be merged into the shared-score table.

### Required metrics

- throughput
- latency distribution where meaningful
- CPU cost
- binary and dependency footprint summary
- profiler-backed hotspot explanation for `iojournal`

## Required Tooling

### Container baseline

- `podman`
- repository-local scripts

### Clang lane

- `clang-tidy`
- `run-clang-tidy.py`
- `scan-build`
- `scan-view`
- `diagtool`
- `clang-check`
- `-ftime-trace`
- `-fproc-stat-report`

### GCC lane

- `gcc -fanalyzer`
- `gcov`
- `gcovr`
- `gcov-tool`
- `gcov-dump`
- `lto-dump`
- `-fprofile-generate`
- `-fprofile-use`
- `-Q --help=optimizers`

### Profiling and debug lane

- `hyperfine`
- `uftrace`
- `valgrind --tool=callgrind`
- `gdb`

## Evidence Pack

The release-blocking evidence pack must include:

- methodology document
- benchmark scenarios and fixture definitions
- raw benchmark artifacts
- normalized result tables
- profiler summaries
- consumer-fit conclusion for `iohttp` and `ioguard`

## Non-Goals

- no C++ library baseline in the RC comparison gate
- no arbitrary ecosystem survey beyond the named Tier 1 and Tier 2 sets
- no publish decision based on benchmark numbers without profiler-backed interpretation
