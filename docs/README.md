# iojournal Documentation

## Purpose

This directory contains the authoritative project documentation for `iojournal`.

## Structure

- `api/`
  Sprint 03 RC API contract workspace for public surface, event model, and lifetime rules
- `architecture/`
  Sprint 04 runtime topology, backpressure, memory ordering, and hot-path constraints
- `en/`
  English stable documentation surface
- `ru/`
  Russian stable documentation surface
- `testing/`
  Sprint 04 concurrency matrix and stress strategy contracts
- `plans/`
  roadmap, master plan, and sprint plans
- `rfc/`
  local RFC mirror and standards navigation documents
- `tmp/`
  research drafts and non-authoritative notes

## Entry Points

- Use `docs/api/README.md` for the current RC public API and event-model contract pack.
- Use `docs/architecture/QUEUE_MODEL.md` for the current RC queue topology and overflow model.
- Use `docs/testing/CONCURRENCY_MATRIX.md` for the current mandatory concurrency validation matrix.
- Use `docs/testing/BENCHMARK_METHODOLOGY.md` for the normalized benchmark and comparison contract.
- Use `docs/testing/PROFILER_WORKFLOW.md` for the profiler and performance-review workflow.
- Start with `docs/en/01-bootstrap-and-rfc-corpus.md` for the current stable bootstrap and RFC corpus contract.
- Use `docs/en/02-file-sink.md` for the current stable RC file sink contract.
- Use `docs/en/03-syslog-contract.md` for the current stable RC syslog formatter and transport contract.
- Use `docs/en/04-tooling-and-agent-workflow.md` for the stable development workflow, Clang toolchain, and agent/skill contract.
- Use `docs/en/05-release-candidate-checklist.md` for the current stable `v0.1.0-rc.1` publication gate.
- Use `docs/en/06-rc1-publish-decision.md` for the current explicit publish or no-publish decision.
- Use `docs/plans/ROADMAP.md` for the delivery path to `v0.1.0-rc.1`.
- Use `docs/rfc/README.md` for the local RFC mirror workflow and corpus maintenance rules.

## Rules

- Treat `docs/en/*` as the authoritative stable documentation set.
- Treat `docs/ru/*` as the translation/adaptation layer.
- Treat `docs/api/*` as the Sprint 03 contract workspace until the same decisions are promoted into numbered stable docs.
- Treat `docs/architecture/*` and `docs/testing/*` as the Sprint 04 contract workspace until the same decisions are promoted into numbered stable docs.
- Treat `docs/plans/*` as the authoritative planning surface.
- Treat `docs/tmp/*` as input material only.
- Run RFC harvesting and verification commands inside the Podman development image.
