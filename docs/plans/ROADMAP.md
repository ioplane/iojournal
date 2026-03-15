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
8. harden the RC and publish evidence

The release gate is sequential:
- Sprint 01 and Sprint 02 freeze the standards baseline.
- Sprint 03 and Sprint 04 freeze public and concurrency contracts.
- Sprint 05 through Sprint 07 are not allowed to invent new protocol scope.
- Sprint 08 verifies only the frozen RC surface.

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
| 08 | Release candidate hardening | examples, verification evidence, packaging, RC checklist |

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

### M4: RC Gate Ready
Exit after Sprint 08 when:
- release notes and changelog are prepared
- examples and verification evidence are published
- release scripts and packaging are exercised
- `v0.1.0-rc.1` checklist is green

## Dependencies

- Sprint 02 depends on Sprint 01 repository/docs baseline.
- Sprint 03 depends on Sprint 02 standards decisions.
- Sprint 04 depends on Sprint 03 public event and error model.
- Sprint 05 depends on Sprint 04 queue and memory-ordering decisions.
- Sprint 06 depends on Sprint 05 core event path.
- Sprint 07 depends on Sprint 02 protocol contracts and Sprint 05 core path.
- Sprint 08 depends on Sprints 05-07 producing runnable artifacts.

Dependency interpretation:
- RFC harvesting and source curation happen in Sprint 01.
- RFC priority, field naming, severity mapping, and timestamp policy are frozen in Sprint 02.
- Any feature that requires a new protocol family after Sprint 02 is post-RC by definition.
- Any feature that requires transport security, exporter-specific schemas, or persistence recovery contracts is post-RC unless it is already named in the sprint sequence above.

## Release Definition

`v0.1.0-rc.1` means:
- first public contract for `ij_*` core logging API
- stable minimum feature set for console, file, and syslog UDP/TCP delivery
- documented limits and non-goals
- reproducible verification evidence
- explicit backlog for post-RC features

`v0.1.0-rc.1` does not require:
- RFC 5425 transport security
- OTLP semantic and transport compatibility
- HEC or Elastic delivery semantics
- multi-process runtime topology
- crash-safe spooling or encryption-at-rest persistence
