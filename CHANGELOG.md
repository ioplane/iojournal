# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Sprint 11D plan `docs/plans/sprints/2026-03-16-sprint-11d-contract-preserving-fast-paths.md` and risk assessment `docs/superpowers/specs/2026-03-16-iojournal-next-optimization-risk-assessment.md` for the next contract-preserving optimization tranche before publication.
- repository-local performance skill `iojournal-performance-optimization` plus updated agent metadata and instructions for the release-blocking perf workflow.
- Sprint 11B plan `docs/plans/sprints/2026-03-16-sprint-11b-event-copy-and-allocation-optimization.md` and Sprint 12 publication plan `docs/plans/sprints/2026-03-16-sprint-12-final-rc-decision-and-publication.md`.
- release-blocking bottleneck and optimization design under `docs/superpowers/specs/2026-03-16-iojournal-bottleneck-and-optimization-design.md`.
- stable publication docs under `docs/en/05-release-candidate-checklist.md`, `docs/en/06-rc1-publish-decision.md`, `docs/ru/05-release-candidate-checklist.md`, and `docs/ru/06-rc1-publish-decision.md`.
- Sprint 11A plan under `docs/plans/sprints/2026-03-16-sprint-11a-bottleneck-and-performance-optimization.md` for bottleneck study, `io_uring` relevance and adoption, SIMD relevance and adoption, and refreshed evidence before publication.
- `scripts/run-podman-perf-lane.sh` as the official Podman launch mode for `io_uring` and ptrace-sensitive profiling lanes.
- `docs/testing/PERF_OPTIMIZATION_GATE.md` and `docs/tmp/perf-analysis/README.md` to freeze the Sprint 11A optimization gate and scratch artifact surface.
- `docs/testing/PERF_HOTSPOT_INVENTORY.md` to freeze the current encode, copy, redaction, validation, queue, and sink-write hotspot inventory before `io_uring` and SIMD relevance work.
- `docs/testing/IO_URING_RELEVANCE.md` plus raw artifact directory `docs/tmp/benchmarks/20260316-143805/` to record the Sprint 11A file-sink `io_uring` accept or reject decision.
- `docs/testing/SIMD_RELEVANCE.md` plus raw artifact directory `docs/tmp/benchmarks/20260316-145708/` to record the Sprint 11A SIMD keep or drop decision for JSON escape scanning, redaction key matching, and UTF-8 validation.
- `clang-perf` and `clang-uftrace` CMake preset lanes plus `scripts/build-uftrace-bench.sh` for Sprint 10 release-facing benchmark and profiler evidence.
- refreshed Sprint 10 raw evidence under `docs/tmp/benchmarks/20260316-114500/` and `docs/tmp/profiling/20260316-114800/`, including `uftrace` and `hyperfine` artifacts for the previously missing shared scenarios.
- Sprint 10 comparison pack under `docs/plans/comparison/`, including Tier 1 functional matrices, `iohttp` and `ioguard` fit analysis, raw artifact registry, and current `iojournal` performance results.
- Benchmark and profiling manifests in `docs/tmp/benchmarks/<RUN_ID>/manifest.md` and `docs/tmp/profiling/<RUN_ID>/summary.md` to make Sprint 10 evidence traceable.
- Tier 1 benchmark harnesses and raw competitor artifacts for `zlog`, `stumpless`, and `tinylog` under `bench/tier1/`, `scripts/run-tier1-benchmarks.sh`, and `docs/tmp/benchmarks/20260316-105219/`.
- normalized Tier 1 `append_file` benchmark coverage for `zlog`, `stumpless`, and `tinylog`, with refreshed competitor artifacts under `docs/tmp/benchmarks/20260316-160500/`.

### Changed

- Benchmark execution now uses the standalone `scripts/benchmarks.py` Python CLI.

- `AGENTS.md`, `CLAUDE.md`, `CODEX.md`, `docs/en/04-tooling-and-agent-workflow.md`, `docs/ru/04-tooling-and-agent-workflow.md`, and the `iojournal-performance-optimization` skill now additionally enforce dual-compiler C23 lanes, forbid mixing GCC and Clang sanitizer or LTO artifacts, require vectorization diagnostics from both compilers for hot-path work, prefer `#pragma omp simd` for portable hints, and keep `restrict` plus floating-point tuning under explicit contract control.
- `docs/plans/comparison/RAW_ARTIFACTS.md`, `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`, and `docs/plans/comparison/PERFORMANCE_RESULTS.md` now reference the Sprint 11D source-of-record evidence pack: local benchmarks in `docs/tmp/benchmarks/20260316-210134`, callgrind/hyperfine profiler pack `docs/tmp/profiling/20260316-210204`, and `uftrace` wall-time packs `docs/tmp/profiling/20260316-210329`, `20260316-210345`, and `20260316-210349`.
- `src/encoders/ij_json_escape_scalar.c`, `src/core/ij_utf8_scalar.c`, and `src/encoders/ij_simd_dispatch.h` now bypass SIMD dispatch for short strings, and the refreshed quiet Sprint 11C source of record is `docs/tmp/benchmarks/20260316-184502-quiet/` with profiler pack `docs/tmp/profiling/20260316-184513/`.
- `src/filters/ij_redact.c` now uses a length-aware and first-byte-aware dispatch for the builtin redaction key set instead of a flat linear scan, while `tests/unit/test_filters.c` now locks the full builtin exact-key set, dotted-suffix behavior, and known non-match cases for Sprint 11C Task 4.
- `src/encoders/ij_json_console.c` now writes RFC3339 timestamps, signed and unsigned decimal values, and `\u00xx` control escapes through fixed-format helpers instead of generic `snprintf` on those hot paths, while `tests/unit/test_console_json.c` and `tests/unit/test_rfc5424.c` now lock exact scalar JSON text and emitted lengths for Sprint 11C Task 3.
- `src/encoders/ij_json_console.c` now uses length-aware builder calls for owned strings, attribute keys, and fixed JSON literals, while `tests/unit/test_console_json.c` and `tests/unit/test_file_sink.c` now lock exact JSON and NDJSON output text plus reported output lengths for Sprint 11C Task 2.
- `src/core/ij_event.c` now uses a fused validation-plus-copy pre-scan for Sprint 11C Task 1, and `tests/unit/test_event.c` now locks the preserved failure behavior for duplicate keys, invalid UTF-8, and aggregate text-size rejection before copy allocation.
- `AGENTS.md`, `CLAUDE.md`, `CODEX.md`, and the local skills now encode the dedicated perf-lane workflow, the current `io_uring` and SIMD decisions, and the next release-blocking optimization target for agents.
- `scripts/run-release-candidate.sh`, `scripts/render-release-notes.sh`, `docs/plans/ROADMAP.md`, and `docs/plans/README.md` now point to the stable Sprint 12 publication checklist and explicit `NO-PUBLISH` RC decision.
- `.github/CODEOWNERS` now assigns repository ownership to `@ioplane/developers`, and the stable publish-decision docs now treat ownership as green while keeping the final remote gate unresolved.
- `AGENTS.md`, `CLAUDE.md`, `CODEX.md`, `docs/plans/ROADMAP.md`, `docs/plans/BACKLOG.md`, and the performance skill now additionally freeze the SIMD policy learned from the latest draft review: scalar-only public ABI, internal-only ISA-specific code and dispatch, `AVX2` plus `NEON` as the RC-active SIMD envelope, and `AVX-512`/`SVE`/`SVE2` deferred to post-RC work.
- `docs/plans/ROADMAP.md`, the master plan, and the sprint index now insert Sprint 11B for event-copy and allocation-churn optimization before the final RC publication sprint.
- `src/core/ij_event.c`, `src/filters/ij_redact.c`, `src/ij_internal.h`, and `tests/unit/test_event.c` now use a packed event-copy text arena and in-place redaction replacement instead of per-string allocations in the normal event-copy path.
- `src/core/ij_logger.c` now bypasses the internal ring in the synchronous logger hot path, `src/sinks/ij_console_sink.c` now writes via `writev(2)` instead of `fwrite` plus `fflush`, `src/filters/ij_redact.c` now precomputes redaction-key lengths and avoids repeated key scans, and `src/encoders/ij_simd_dispatch.h` now caches x86 AVX2 runtime detection; the refreshed `clang-perf` local source of record is `docs/tmp/benchmarks/20260316-160705/`.
- `docs/plans/ROADMAP.md`, `docs/plans/README.md`, `docs/plans/sprints/README.md`, `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`, and `docs/plans/BACKLOG.md` to insert the new mandatory pre-publication optimization sprint and to narrow the post-RC `io_uring` backlog item to broader backend expansion only.
- `AGENTS.md`, `CLAUDE.md`, `CODEX.md`, `docs/en/04-tooling-and-agent-workflow.md`, `docs/ru/04-tooling-and-agent-workflow.md`, and `docs/testing/PROFILER_WORKFLOW.md` now distinguish the default Podman lane from the dedicated perf/io_uring launch mode.
- `docs/testing/BENCHMARK_METHODOLOGY.md` and `docs/testing/PROFILER_WORKFLOW.md` now freeze the shared optimization scenario set and profiler evidence rules for Sprint 11A.
- `scripts/run-profiler-review.sh`, `docs/testing/PROFILER_WORKFLOW.md`, and the stable workflow docs now use `clang-perf` as the release-facing baseline and document the dedicated `clang-uftrace` lane for `uftrace` evidence.
- Sprint 10 comparison docs now use the authoritative `clang-perf` local baseline `20260316-114500` and profiler pack `20260316-114800`, and they record that toolchain parity, preset parity, profiler parity, and repeatability framing are closed.
- Sprint 10 comparison docs and raw artifact registry now promote the fresh Sprint 11A Task 3 baseline `20260316-140816` plus the paired profiler pack `20260316-140903`, `20260316-140954`, `20260316-141102`, and `20260316-141113` as the active bottleneck-confirmation source of record.
- Sprint 11A Task 4 now keeps the synchronous file sink as the default RC path and records that the explicit `io_uring` file backend is rejected for the current RC-active file row after the dedicated perf-lane benchmark proved materially slower than the synchronous baseline.
- Sprint 11A Task 6 now keeps `x86_64 AVX2` active only for JSON escape scanning and UTF-8 validation, rejects SIMD redaction matching for the RC path, and keeps `arm64 NEON` out of the RC-active dispatch policy until arm64 measurements exist.
- Sprint 11B now promotes the refreshed evidence pack `20260316-160705`, `20260316-150320`, `20260316-164059`, `20260316-164602`, `20260316-164658`, and `20260316-164607` as the active source of record and hands the release line off to Sprint 12 publication hardening.
- `scripts/run-benchmarks.sh` now records scenario status alongside raw TSV outputs.
- `scripts/run-profiler-review.sh` now supports non-interactive `auto` mode and emits callgrind summaries for Sprint 10 evidence.
- Sprint 10 local benchmark coverage now includes `medium_message`, `medium_message_with_metadata`, and `contention_mpsc` in the shared scenario set.
- Sprint 10 comparison docs now include the first normalized Tier 1 shared-scenario results and consumer-fit interpretation for `zlog`, `stumpless`, and `tinylog`.
- `scripts/run-benchmarks.sh` now aligns local file benchmark iterations with the shared Sprint 10 comparison contract by default, and the refreshed local baseline is recorded under `docs/tmp/benchmarks/20260316-160650/`.
- Sprint 10 methodology and performance docs now treat normalized file append as a shared comparison slice, while documenting the current `iojournal` `append_ndjson` artifact label and the current `zlog` stdout-to-file normalization caveat.

 - Sprint 09 benchmark harness under `bench/`, including `bench_hot_path`, `bench_file_sink`, `bench_syslog_udp`, raw artifact generation via `scripts/run-benchmarks.sh`, and profiler entrypoints via `scripts/run-profiler-review.sh`.
- Sprint 09 benchmark methodology and profiler workflow contracts under `docs/testing/BENCHMARK_METHODOLOGY.md` and `docs/testing/PROFILER_WORKFLOW.md`.
- release-blocking performance and comparison planning pack, including `docs/plans/2026-03-16-iojournal-functional-and-performance-comparison-plan.md` and Sprint 09-11 plans for benchmark methodology, competitor comparison, and final RC publication.
- stable English and Russian workflow document pair under `docs/en/04-tooling-and-agent-workflow.md` and `docs/ru/04-tooling-and-agent-workflow.md` covering Podman execution, Clang 22 tooling, profiling/debug stack, skills, and agent instruction roles.
- Sprint 07 RFC 5424 syslog formatter, RFC 5426 UDP sink, RFC 6587 TCP octet-counting sink, dedicated unit coverage, and runnable syslog examples.
- stable English and Russian Sprint 07 syslog documents under `docs/en/03-syslog-contract.md` and `docs/ru/03-syslog-contract.md`.
- Sprint 05 Task 1 build surface for `iojournal`, including the first CMake project, presets, public headers, internal header, version helpers, logger lifecycle stubs, and Unity smoke test wiring.
- Sprint 05 Task 2 bounded core runtime slice for `iojournal`, including event validation and deep-copy helpers, bounded ring helpers, level filtering, redaction helpers, RFC3339 timestamp formatting, JSON console encoding, console sink output, and dedicated unit coverage for event, queue, filter, and console-path behavior.
- Sprint 05 Task 3 runnable `examples/basic_console.c` target built inside the default containerized CMake workflow.
- Sprint 06 Task 1 NDJSON file sink path for `iojournal`, including public file-sink definitions, append-only file persistence, and dedicated unit coverage for file output behavior.
- Sprint 06 Task 2 file rotation and retention baseline, plus stable English and Russian file sink documents under `docs/en/02-file-sink.md` and `docs/ru/02-file-sink.md`.
- Sprint 06 Task 3 runnable `examples/file_sink.c` target for RC NDJSON output.
- Sprint 03 RC API contract pack under `docs/api/`, covering public API surface, error model, event model, attribute model, ownership rules, examples, and test obligations.
- Sprint 04 RC concurrency contract pack under `docs/architecture/` and `docs/testing/`, covering queue topology, backpressure, memory ordering, hot-path limits, concurrency matrix, and stress strategy.
- RFC corpus planning docs `docs/rfc/CANONICAL_LIST.md` and `docs/rfc/FEATURE_MATRIX.md` for Sprint 02 standards freeze.
- RFC contract docs `docs/rfc/TIMESTAMP_POLICY.md`, `docs/rfc/FIELD_NAMING_POLICY.md`, `docs/rfc/JSON_NDJSON_CONTRACT.md`, `docs/rfc/REDACTION_POLICY.md`, and `docs/rfc/SYSLOG_CONTRACT.md` for Sprint 02 protocol freeze.
- RFC defer-and-divergence note `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` for RC scope lock.
- bootstrap repository baseline for `iojournal`, including root agent instructions, GitHub workflow scaffolding, and release helper scripts
- repository-local skills for architecture, coding standards, RFC guidance, C23 usage, and repository conventions
- roadmap, master plan, and sprint plan set for delivery to `v0.1.0-rc.1`
- Sprint 01 documentation baseline with docs indexes, RFC source catalog, RFC priority map, and RFC harvesting workflow notes
- first stable numbered documentation pair for the Sprint 01 bootstrap and RFC corpus surface
- Podman development image baseline now includes `jq` for container-only workflows

- `CMakeLists.txt`, `CMakePresets.json`, `scripts/quality.sh`, and `scripts/run-coverage.sh` to add the Sprint 09 benchmark build matrix, GCC analyzer lane, and dual Clang/GCC coverage reporting with `gcovr`.
- `docs/README.md`, `docs/en/04-tooling-and-agent-workflow.md`, `docs/ru/04-tooling-and-agent-workflow.md`, and `docs/plans/sprints/2026-03-16-sprint-09-benchmark-harness-and-methodology.md` to expose and mark completed the Sprint 09 methodology and toolchain surface.
- `docs/plans/ROADMAP.md`, `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`, `docs/plans/README.md`, `docs/plans/sprints/README.md`, and `docs/plans/BACKLOG.md` to make functional and performance comparison mandatory before `v0.1.0-rc.1`.
- `docs/en/04-tooling-and-agent-workflow.md`, `docs/ru/04-tooling-and-agent-workflow.md`, `AGENTS.md`, `CLAUDE.md`, and `CODEX.md` to add the GCC 15 and `gcovr` tooling lane next to the Clang 22 and profiler stack.
- `docs/README.md`, `docs/en/README.md`, and `docs/ru/README.md` to expose the stable tooling and agent workflow document.
- `docs/README.md`, `docs/en/README.md`, and `docs/ru/README.md` to expose the stable Sprint 07 syslog document in the numbered docs surface.
- `docs/plans/sprints/2026-03-14-sprint-07-syslog-protocol-delivery.md` to mark Sprint 07 formatter, transport, and evidence tasks as completed.
- `scripts/quality.sh` now has a real CMake/analyzer execution path, external Unity analyzer filtering, and example execution checks once the new Sprint 05 build surface is present.
- `scripts/quality.sh` now executes both console and file sink examples and validates the expected emitted output in the containerized quality gate.
- `docs/plans/sprints/2026-03-14-sprint-05-core-runtime-mvp.md` to mark all Sprint 05 Task 1-3 implementation steps as completed.
- `docs/README.md`, `docs/en/README.md`, and `docs/ru/README.md` to expose the stable file sink document in the numbered docs surface.
- `docs/plans/BACKLOG.md` to add the deferred post-RC CodeChecker hardening track for maximal C23 analysis coverage.
- `docs/README.md`, `docs/en/README.md`, `docs/ru/README.md`, `docs/plans/ROADMAP.md`, and `docs/plans/sprints/README.md` to expose the new Sprint 03 API contract workspace.
- `docs/plans/sprints/2026-03-14-sprint-03-public-api-and-data-model.md` to mark Sprint 03 API and data-model tasks as completed.
- `docs/README.md`, `docs/plans/ROADMAP.md`, and `docs/plans/sprints/README.md` to expose the new Sprint 04 concurrency contract workspace.
- `docs/plans/sprints/2026-03-14-sprint-04-buffering-and-concurrency-spec.md` to mark Sprint 04 queue, ordering, and stress-plan tasks as completed.
- `docs/rfc/README.md` and `docs/rfc/PRIORITY.md` to align the RFC corpus workflow with the new canonical list and deferred-only classification.
- `docs/plans/sprints/2026-03-14-sprint-02-rfc-corpus-and-protocol-contracts.md` to mark Task 1 and Task 2 as completed.
- `docs/plans/BACKLOG.md` to link deferred protocol items back to the Sprint 02 defer rationale.

### Removed

- Removed the `scripts/run-benchmarks.sh` benchmark entrypoint.

### Fixed

- Datatracker query handling in `scripts/rfc-scraper.py` to use supported document filters and container-validated RFC registry generation
