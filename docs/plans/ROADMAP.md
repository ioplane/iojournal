# iojournal Roadmap To v0.1.0-rc.1

## Goal

Ship the first public release candidate of `iojournal` as a C23 structured logging library with:
- bounded hot path
- repository and standards baseline
- JSON console sink
- NDJSON file sink
- RFC 5424 formatter
- RFC 5426 UDP syslog sink
- RFC 6587 TCP syslog sink
- functional comparison against server-grade C logging alternatives
- performance evidence and profiler-backed review
- release evidence and examples

Explicit non-goals for `v0.1.0-rc.1`:
- OTLP exporter
- syslog over TLS
- Splunk HEC
- Elastic Bulk
- shared-memory multi-process bus
- disk spool queues
- encryption at rest

These items are backlog-only. They are not release-candidate blockers, and they must not expand Sprint 01-08 scope.

## Delivery Strategy

Use a protocol-first program:
1. build the repository and research baseline
2. assemble the RFC and standards corpus
3. freeze the public contracts
4. specify concurrency and backpressure
5. implement the runtime core
6. add local persistence
7. add syslog delivery
8. harden the RC infrastructure and evidence pipeline
9. build the benchmark and comparison harness
10. execute functional and performance comparison against the Tier 1 set
11. execute the release-blocking bottleneck and optimization sprint
12. execute the release-blocking structural hot-path redesign sprint
13. make the final publish decision from feature, comparison, and optimization evidence

The release gate is sequential:
- Sprint 01 and Sprint 02 freeze the standards baseline.
- Sprint 03 and Sprint 04 freeze public and concurrency contracts.
- Sprint 05 through Sprint 07 are not allowed to invent new protocol scope.
- Sprint 08 hardens only the frozen RC surface.
- Sprint 09 and Sprint 10 add mandatory benchmark and comparison evidence without inventing new sink scope.
- Sprint 11A proves or rejects the first optimization surfaces and refreshes the evidence pack.
- Sprint 11B applies the next measured optimization slice for event copy and allocation churn without widening RC scope.
- Sprint 11C applies the next structural hot-path redesign slice without changing the public contract.
- Sprint 12 makes the publish decision only after feature, comparison, and optimization evidence are all green.

## Sprint Sequence

| Sprint | Theme | Primary Outcome |
|---|---|---|
| 01 | Repository and RFC bootstrap | stable planning/docs/release baseline plus Python-driven RFC source list |
| 02 | RFC corpus and protocol contracts | canonical standards map and feature matrix |
| 03 | Public API and data model | first RC public contract and event schema under `docs/api/` |
| 04 | Buffering and concurrency spec | queue model, ordering rules, and stress matrix under `docs/architecture/` and `docs/testing/` |
| 05 | Core runtime MVP | runnable core with bounded event path and JSON console output |
| 06 | File sink and persistence | NDJSON file sink with rotation/retention baseline |
| 07 | Syslog protocol delivery | RFC 5424/5426/6587 implementation and interoperability evidence |
| 08 | RC infrastructure and evidence hardening | release scripts, artifacts, docs checklists, and publish gate scaffolding |
| 09 | Benchmark harness and methodology | benchmark scenarios, profiling stack, measurement rules, and raw artifact workflow |
| 10 | Functional and performance comparison | Tier 1 comparison against `zlog`, `stumpless`, and `tinylog`, plus Tier 2 appendix |
| 11A | Bottleneck and performance optimization | release-blocking bottleneck study, `io_uring` relevance and adoption, SIMD relevance and adoption, refreshed evidence |
| 11B | Event copy and allocation churn optimization | release-blocking event-copy, redaction-allocation, and evidence refresh work |
| 11C | Structural hot-path redesign | release-blocking fused validation/copy, length-aware encode, formatting, and redaction-dispatch work |
| 11D | Contract-preserving fast paths | release-blocking fixed-fragment JSON shaping, event-copy tightening, selective redaction reassessment, and refreshed evidence |
| 12 | Final RC decision and publication | publish/no-publish decision, final release evidence, and tag/release preparation |
| 13 | Audit bug fixes and test hardening | critical/high-severity bug fixes from independent audit, concurrency tests, UTF-8 tests, SIMD equivalence tests, encoder overflow tests |
| 14 | Shell script modernization | all 14 scripts rewritten to Bash 5.1+ with shellcheck 0.11.0 compliance, shared library, shellcheck quality gate step |

## Current Execution Status

- Sprint 01 through Sprint 12 Task 0 are complete.
- Active source-of-record artifacts are now:
  - local benchmark source of record [`20260316-210134`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134)
  - Tier 1 comparison [`20260316-150320`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320)
  - callgrind and repeatability pack [`20260316-210204`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204)
  - `uftrace` companion runs [`20260316-210329`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210329), [`20260316-210345`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210345), and [`20260316-210349`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210349)
- Sprint 12 publication hardening is unblocked from a local evidence perspective and proceeds to final gate execution.
- The current publication decision for `v0.1.0-rc.1` remains `NO-PUBLISH`; see [`docs/en/06-rc1-publish-decision.md`](/opt/projects/repositories/iojournal/docs/en/06-rc1-publish-decision.md).

## Milestones

### M1: Standards Baseline Ready
Exit after Sprint 02 when:
- RFC list is frozen for the RC
- standards priority map exists
- field naming and timestamp policy are fixed
- Python RFC harvesting workflow is documented and reproducible

### M2: Core Contract Ready
Exit after Sprint 04 when:
- public API draft is frozen
- ownership and error model are documented
- `docs/api/*` defines the RC contract pack
- queue and overflow behavior are specified
- `docs/architecture/*` and `docs/testing/*` define the RC concurrency contract pack

### M3: Feature Surface Ready
Exit after Sprint 07 when:
- JSON console sink works
- file sink works
- syslog UDP/TCP delivery works
- golden and negative tests exist for core protocol rules

### M4: RC Infrastructure Ready
Exit after Sprint 08 when:
- release notes and changelog are prepared
- examples and verification evidence pipeline is in place
- release scripts and packaging are exercised
- RC decision inputs are measurable

### M5: Comparison Evidence Ready
Exit after Sprint 10 when:
- benchmark methodology is frozen
- Tier 1 functional fit matrix exists
- Tier 1 performance comparison results exist
- profiler-backed conclusions for `iojournal` are published
- authoritative comparison artifacts exist under `docs/plans/comparison/`

### M6: RC Publish Ready
Exit after Sprint 12 when:
- release checklist is green
- comparison gate is green
- optimization gate is green
- publish decision is explicit and evidence-backed

## Dependencies

- Sprint 02 depends on Sprint 01 repository/docs baseline.
- Sprint 03 depends on Sprint 02 standards decisions.
- Sprint 04 depends on Sprint 03 public event and error model.
- Sprint 05 depends on Sprint 04 queue and memory-ordering decisions.
- Sprint 06 depends on Sprint 05 core event path.
- Sprint 07 depends on Sprint 02 protocol contracts and Sprint 05 core path.
- Sprint 08 depends on Sprints 05-07 producing runnable artifacts.
- Sprint 09 depends on Sprint 08 release-evidence scaffolding and the runnable feature surface.
- Sprint 10 depends on Sprint 09 benchmark harness and comparison methodology.
- Sprint 11A depends on Sprint 09 and Sprint 10, plus the runnable RC feature surface.
- Sprint 11B depends on Sprint 11A and the refreshed hotspot evidence.
- Sprint 11C depends on Sprint 11B and the refreshed profiler evidence.
- Sprint 11D depends on Sprint 11C and the refreshed risk-assessment gate.
- Sprint 12 depends on Sprint 08 through Sprint 11D.

Dependency interpretation:
- RFC harvesting and source curation happen in Sprint 01.
- RFC priority, field naming, severity mapping, and timestamp policy are frozen in Sprint 02.
- Any feature that requires a new protocol family after Sprint 02 is post-RC by definition.
- Any feature that requires transport security, exporter-specific schemas, or persistence recovery contracts is post-RC unless it is already named in the sprint sequence above.
- Comparison work before the RC is limited to pure C logging libraries that are plausible for `iohttp` and `ioguard`.
- Release-blocking optimization work before the RC is limited to proven bottlenecks with explicit before/after evidence and must not widen protocol scope beyond the current RC surface.
- Sprint 11B is limited to event copy, owned-string allocation churn, and redaction replacement churn; it must not introduce new sink families, queue topologies, or async worker semantics.
- Sprint 11C is limited to structural hot-path redesign inside the current synchronous contract; it must not widen sink scope, async semantics, or the public ABI.
- Sprint 11D is limited to contract-preserving fast paths; it must not introduce common-shape dual behavior unless earlier slices fail to close the release-relevant gap.
- SIMD remains an internal implementation detail for the RC line: keep the public ABI scalar-only, keep ISA-specific code isolated from public headers, and keep `AVX-512` and `SVE/SVE2` out of RC scope unless a new measured plan explicitly reopens them.

## Mandatory Handoff Rules

- Do not change sprint sequence, publish status, or scope without synchronously updating:
  - `docs/plans/ROADMAP.md`
  - `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
  - `docs/plans/sprints/README.md`
  - the currently active sprint plan in `docs/plans/sprints/`
  - `docs/en/06-rc1-publish-decision.md` and `docs/ru/06-rc1-publish-decision.md` when gate state changes
- Before handoff for another AI, the following commands are mandatory:
  - `python3 scripts/lint-docs.py`
  - `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
  - `bash scripts/run-release-candidate.sh`
  - `bash scripts/build-release-assets.sh v0.1.0-rc.1`
  - `bash scripts/render-release-notes.sh`
- Keep evidence surfaces synchronized if performance evidence changes:
  - `docs/plans/comparison/PERFORMANCE_RESULTS.md`
  - `docs/plans/comparison/RAW_ARTIFACTS.md`
  - `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`
  - `docs/testing/IO_URING_RELEVANCE.md`
  - `docs/testing/SIMD_RELEVANCE.md`

## Release Definition

`v0.1.0-rc.1` means:
- first public contract for `ij_*` core logging API
- stable minimum feature set for console, file, and syslog UDP/TCP delivery
- functional comparison against `zlog`, `stumpless`, and `tinylog`
- performance comparison methodology, raw artifacts, and profiler-backed review
- bottleneck study and optimization evidence, including `io_uring` and SIMD accept/reject decisions
- event-copy and allocation-churn optimization evidence
- structural hot-path redesign evidence
- consumer-fit conclusions for `iohttp` and `ioguard`
- raw artifact registry for benchmark and profiling evidence
- documented limits and non-goals
- reproducible verification evidence
- explicit backlog for post-RC features

`v0.1.0-rc.1` does not require:
- RFC 5425 transport security
- OTLP semantic and transport compatibility
- HEC or Elastic delivery semantics
- multi-process runtime topology
- crash-safe spooling or encryption-at-rest persistence
