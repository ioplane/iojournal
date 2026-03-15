# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
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

### Changed
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

### Fixed

- Datatracker query handling in `scripts/rfc-scraper.py` to use supported document filters and container-validated RFC registry generation
