# iojournal English Documentation

## Scope

This directory contains the stable English documentation set for `iojournal`.

## Stable Documents

- `01-bootstrap-and-rfc-corpus.md`
  Sprint 01 bootstrap contract for RFC harvesting, local RFC mirror outputs, and stable docs initialization.
- `02-file-sink.md`
  Sprint 06 RC file sink contract for NDJSON persistence, rotation, retention, and failure handling.
- `03-syslog-contract.md`
  Sprint 07 RC syslog contract for RFC 5424 formatting, RFC 5426 UDP delivery, and RFC 6587 TCP framing.
- `04-tooling-and-agent-workflow.md`
  stable workflow contract for Podman execution, Clang 22 tooling, profiling/debug stack, and agent/skill responsibilities.
- `05-release-candidate-checklist.md`
  stable publication checklist for `v0.1.0-rc.1`, including local quality, comparison, optimization, and repository-governance gates.
- `06-rc1-publish-decision.md`
  explicit publish or no-publish decision record for `v0.1.0-rc.1`.

## Current Navigation

- Read `01-bootstrap-and-rfc-corpus.md` before changing `docs/rfc/*` or Sprint 01 planning artifacts.
- Read `02-file-sink.md` before changing the RC file sink path, persistence tests, or retention behavior.
- Read `03-syslog-contract.md` before changing RFC 5424 formatting, syslog transport behavior, or Sprint 07 evidence.
- Read `04-tooling-and-agent-workflow.md` before changing container workflow, Clang tooling, profiling stack, skills, or agent policy integration.
- Read `05-release-candidate-checklist.md` before changing RC publication criteria, release-gate inputs, or Sprint 12 evidence requirements.
- Read `06-rc1-publish-decision.md` before claiming the repository is ready for public `v0.1.0-rc.1` publication.
- Use `docs/api/README.md` for the current Sprint 03 RC API contract pack; it is a technical workspace, not yet a numbered stable document.
- Use `docs/rfc/README.md` for corpus maintenance commands and artifact ownership.
- Use `docs/plans/ROADMAP.md` for milestone and dependency context.

## Next Documents

- architecture and scope
- API and data model
- release candidate verification notes
- post-RC hardening notes

## Related Directories

- `docs/plans/`
- `docs/rfc/`
