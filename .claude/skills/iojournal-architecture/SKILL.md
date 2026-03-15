---
name: iojournal-architecture
description: Use when implementing or refactoring core queues, encoders, sinks, filters, public APIs, backpressure policy, or io_uring-sensitive logging paths in iojournal. Mandatory for architecture-sensitive work in `include/iojournal/`, `src/`, and related tests.
---

# Iojournal Architecture

## Overview

Use this skill to keep `iojournal` aligned with its logging-library boundaries: bounded hot path, preallocated transport between producers and consumers, and strict separation between record creation, encoding, and delivery.

Read `docs/plans/2026-03-10-iojournal-c23-architecture-plan.md` before changing architecture-sensitive code.

## Core Model

- Treat the library as a bounded pipeline:
  - event production
  - queueing and handoff
  - filtering and redaction
  - encoding
  - sink delivery
- Keep `ij_log` and equivalent hot-path entry points allocation-free.
- Preallocate buffers and queue slots before traffic starts.
- Preserve clear ownership between core runtime, encoders, sinks, and filters.

## Architectural Boundaries

- `core`
  - context, runtime, queues, ring buffers, worker handoff, bounded memory
- `filters`
  - level filtering, sampling, rate limiting, redaction policy
- `encoders`
  - JSON, RFC 5424, NDJSON, OTLP payload shaping
- `sinks`
  - console, file, syslog, OTLP, Splunk HEC, Elastic bulk
- Higher-level integrations (`iohttp`, `liboas`, tracing adapters) are consumers of the logging API, not reasons to blur core ownership.

## Invariants

- Zero heap allocation in the hot path.
- Bounded queue sizes and explicit overflow policy.
- Backpressure is a designed behavior, not an accident.
- Redaction is enabled by default for sensitive fields.
- Sink/network reliability concerns must not leak blocking behavior into producers.
- `io_uring` optimizations must respect buffer lifetime until completion notification.

## Workflow

When changing architecture-sensitive code:
1. Decide which layer owns the change.
2. Confirm the change belongs in `iojournal`, not a consumer integration.
3. Check whether the hot path stays bounded and allocation-free.
4. Verify queueing, redaction, and sink ownership still match the module boundary.
5. Update tests and the architecture skill references if the boundary shifts.

## References

- `references/module-map.md`
