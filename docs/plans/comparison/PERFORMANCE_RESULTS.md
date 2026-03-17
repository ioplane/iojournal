# Sprint 10 Through Sprint 11D Performance Results

## Method Contract

- Measurement rules are inherited from [`BENCHMARK_METHODOLOGY.md`](/opt/projects/repositories/iojournal/docs/testing/BENCHMARK_METHODOLOGY.md).
- Shared scenarios and capability-specific scenarios remain separate.
- Active source of record:
  - `iojournal` local benchmark run [`20260316-210134`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134)
  - Tier 1 competitor benchmark run [`20260316-150320`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320)
  - callgrind and repeatability pack [`20260316-210204`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204)
  - `uftrace` companion runs [`20260316-210329`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210329), [`20260316-210345`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210345), and [`20260316-210349`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210349)
- The release-facing local baseline uses the `clang-perf` preset, while `uftrace` evidence uses the `clang-uftrace` preset and dedicated perf lane.
- Capability-specific rows remain appendix-only and must not be folded into the shared score table.
- Sprint 11D focused on hot-path frontier after the shared release scenarios were aligned.

## Current Measured Results

### Shared Scenarios

| Library | Scenario | Iterations | Elapsed ns | ns/op | Artifact |
| --- | --- | --- | --- | --- | --- |
| `iojournal` | `disabled_level` | `50000` | `396000` | `7.92` | [`bench_hot_path.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134/bench_hot_path.tsv) |
| `iojournal` | `enabled_console` | `50000` | `44680898` | `893.62` | [`bench_hot_path.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134/bench_hot_path.tsv) |
| `iojournal` | `medium_message` | `50000` | `39035025` | `780.70` | [`bench_hot_path.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134/bench_hot_path.tsv) |
| `iojournal` | `medium_message_with_metadata` | `50000` | `47550071` | `951.00` | [`bench_hot_path.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134/bench_hot_path.tsv) |
| `iojournal` | `contention_mpsc` | `50000` | `17786353` | `355.73` | [`bench_hot_path.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134/bench_hot_path.tsv) |
| `iojournal` | `append_ndjson` | `50000` | `32426534` | `648.53` | [`bench_file_sink.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134/bench_file_sink.tsv) |
| `zlog` | `disabled_level` | `50000` | `707167` | `14.14` | [`bench_zlog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_zlog.tsv) |
| `zlog` | `enabled_console` | `50000` | `28651292` | `573.03` | [`bench_zlog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_zlog.tsv) |
| `zlog` | `medium_message` | `50000` | `22575043` | `451.50` | [`bench_zlog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_zlog.tsv) |
| `zlog` | `medium_message_with_metadata` | `50000` | `23536407` | `470.73` | [`bench_zlog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_zlog.tsv) |
| `zlog` | `contention_mpsc` | `50000` | `9153178` | `183.06` | [`bench_zlog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_zlog.tsv) |
| `zlog` | `append_file` | `50000` | `98195050` | `1963.90` | [`bench_zlog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_zlog.tsv) |
| `stumpless` | `disabled_level` | `50000` | `5411977` | `108.24` | [`bench_stumpless.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_stumpless.tsv) |
| `stumpless` | `enabled_console` | `50000` | `51603229` | `1032.06` | [`bench_stumpless.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_stumpless.tsv) |
| `stumpless` | `medium_message` | `50000` | `49333951` | `986.68` | [`bench_stumpless.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_stumpless.tsv) |
| `stumpless` | `medium_message_with_metadata` | `50000` | `57296815` | `1145.94` | [`bench_stumpless.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_stumpless.tsv) |
| `stumpless` | `contention_mpsc` | `50000` | `53917796` | `1078.36` | [`bench_stumpless.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_stumpless.tsv) |
| `stumpless` | `append_file` | `50000` | `52071752` | `1041.44` | [`bench_stumpless.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_stumpless.tsv) |
| `tinylog` | `disabled_level` | `50000` | `324313` | `6.49` | [`bench_tinylog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_tinylog.tsv) |
| `tinylog` | `enabled_console` | `50000` | `68416160` | `1368.32` | [`bench_tinylog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_tinylog.tsv) |
| `tinylog` | `medium_message` | `50000` | `59902325` | `1198.05` | [`bench_tinylog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_tinylog.tsv) |
| `tinylog` | `medium_message_with_metadata` | `50000` | `66643668` | `1332.87` | [`bench_tinylog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_tinylog.tsv) |
| `tinylog` | `contention_mpsc` | `50000` | `44258335` | `885.17` | [`bench_tinylog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_tinylog.tsv) |
| `tinylog` | `append_file` | `50000` | `42286060` | `845.72` | [`bench_tinylog.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320/bench_tinylog.tsv) |

Interpretation:

- `disabled_level` remains fastest with `tinylog` and now clearly ahead of both `zlog` and `stumpless`.
- Sprint 11D materially improved every active enabled shared row versus the Sprint 11C source: `enabled_console` from `1059.62` to `893.62 ns/op`, `medium_message` from `819.13` to `780.70 ns/op`, `medium_message_with_metadata` from `1059.55` to `951.00 ns/op`, `contention_mpsc` from `403.21` to `355.73 ns/op`, and `append_ndjson` from `822.84` to `648.53 ns/op`.
- `iojournal` still trails `zlog` on every enabled shared scenario except `disabled_level` (where it is ahead).
- For enabled shared paths, `iojournal` is now ahead of `stumpless` and `tinylog` on `enabled_console`, `medium_message`, `medium_message_with_metadata`, `contention_mpsc`, and `append_ndjson`.
- `append_ndjson` remains the best measured shared file row in the current Tier 1 comparison set.
- Sink-write behavior remains dominated by shaping and copy/validation stages, while raw file write (`ij_file_sink_write` + sync write) is no longer the primary driver.

### Capability-Specific Scenarios

| Library | Scenario | Iterations | Elapsed ns | ns/op | Artifact |
| --- | --- | --- | --- | --- | --- |
| `iojournal` | `udp_loopback` | `5000` | `20825816` | `4165.16` | [`bench_syslog_udp.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134/bench_syslog_udp.tsv) |

Interpretation:

- `udp_loopback` remains appendix-only.
- It is useful for syslog-path regression tracking but must not be merged into the shared-score table.

## Repeatability Framing

The active repeatability slice uses `hyperfine` against the `clang-perf` binaries in the profiler run [`20260316-210204`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204).

| Scenario | Mean | Range | Artifact |
| --- | --- | --- | --- |
| `medium_message_with_metadata` | `3.7 ± 0.5 ms` for `3000` iterations | `3.3 ms .. 6.3 ms` | [`hyperfine-bench_hot_path-medium_message_with_metadata.md`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204/hyperfine-bench_hot_path-medium_message_with_metadata.md) |
| `contention_mpsc` | `1.7 ± 0.4 ms` for `3000` iterations | `1.2 ms .. 7.8 ms` | [`hyperfine-bench_hot_path-contention_mpsc.md`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204/hyperfine-bench_hot_path-contention_mpsc.md) |
| `append_file` | `3.0 ± 0.6 ms` for `3000` iterations | `2.6 ms .. 9.4 ms` | [`hyperfine-bench_file_sink-append_ndjson.md`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204/hyperfine-bench_file_sink-append_ndjson.md) |

Rules:

- These rows are repeatability framing, not replacements for the primary shared-scenario score table.
- The short-runtime warning from `hyperfine` remains expected for these narrow microbenchmarks and must be carried with the artifacts.

## Profiler-Backed Interpretation

### `medium_message_with_metadata`

Sources:
- [`callgrind-bench_hot_path-medium_message_with_metadata.summary.txt`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204/callgrind-bench_hot_path-medium_message_with_metadata.summary.txt)
- [`report.txt`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210329/uftrace-bench_hot_path-medium_message_with_metadata/report.txt)

Observed hotspots:

- `ij_event_copy_from_input` dominates at `76.20%` of instruction stream.
- `ij_scan_string_field_copy` remains the largest single internal loop at `34.61%`.
- `ij_json_console_encode` is second frontier at `18.01%`.
- Redaction and timestamp validation are still persistent but secondary (`ij_key_should_redact_n` at `5.84%`, `ij_format_timestamp_rfc3339` at `7.87%`, `ij_builder_append_json_string_known_escape` at `4.35%`).
- `uftrace` confirms ordering by wall time: `ij_event_copy_from_input` (~14.26 ms), `ij_json_console_encode` (~14.08 ms), `ij_builder_append_json_string_known_escape` (~3.67 ms).

Conclusion:

- The metadata-heavy shared row remains copy, scan, and encode heavy.
- Sink write and scheduling are visible but not primary in the metadata-heavy row.

### `contention_mpsc`

Source:
- [`callgrind-bench_hot_path-contention_mpsc.summary.txt`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204/callgrind-bench_hot_path-contention_mpsc.summary.txt)
- [`report.txt`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210345/uftrace-bench_hot_path-contention_mpsc/report.txt)

Observed hotspots:

- `ij_event_copy_from_input` is `75.50%`.
- `ij_scan_string_field_copy` remains around `34.27%`.
- `ij_json_console_encode` remains around `17.83%`.
- Redaction/validation is non-trivial but no longer top-2.
- `linux:schedule` is visible in wall time but still below copy+encode in core cost.

Conclusion:

- Contention row remains bound by copy/scan/encode.
- Scheduler pressure persists but does not dominate the current primary row.

### `append_ndjson`

Sources:
- [`callgrind-bench_file_sink-append_ndjson.summary.txt`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204/callgrind-bench_file_sink-append_ndjson.summary.txt)
- [`report.txt`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210349/uftrace-bench_file_sink-append_ndjson/report.txt)

Observed hotspots:

- `ij_event_copy_from_input` is `70.89%` and `ij_scan_string_field_copy` is `32.82%`.
- `ij_ndjson_encode` and `ij_json_console_encode` together are about `38.2%`.
- `ij_file_sink_write` plus `ij_file_sink_sync_write` remain lower than encode/copy, at roughly `4.15%` and `3.14%` respectively.

Conclusion:

- The normalized file-row is still encode-heavy and copy-heavy, with sink write still not the primary frontier.

## Release-Blocking Synthesis

- Toolchain parity is satisfied in the current Podman image: `diagtool`, `hyperfine`, and `uftrace` are present, and `clang-perf` and `clang-uftrace` presets are validated.
- The local source-of-record benchmark is now `20260316-210134`; the active comparator remains `20260316-150320`.
- The profiler evidence source of record is now [`20260316-210204`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204) with `uftrace` companions `20260316-210329`, `20260316-210345`, and `20260316-210349`.
- Quality and release tooling remain passing after this 11D refresh.
- `iojournal` now leads `stumpless` and `tinylog` across all enabled shared rows currently measured and compared, but `zlog` still holds the best enabled throughput frontier.
- Sprint 11D is now the active optimization evidence slice. Publication hardening remains in Sprint 12.
