# RC Hot-Path Limits For v0.1.0-rc.1

[![C23](https://img.shields.io/badge/C-23-blue)](https://en.cppreference.com/w/c/23)
[![RFC 8259](https://img.shields.io/badge/RFC-8259-json-orange)](https://www.rfc-editor.org/rfc/rfc8259.html)
[![OWASP Logging](https://img.shields.io/badge/OWASP-Logging%20Cheat%20Sheet-red)](https://cheatsheetseries.owasp.org/cheatsheets/Logging_Cheat_Sheet.html)

## Scope

- Freeze the producer-path constraints that later code and reviews must enforce.
- Connect Sprint 03 event bounds to the runtime implementation budget.
- Make cacheline and copy limits explicit enough for profiling and test work.

## Required Producer-Path Limits

- `ij_logger_log` must perform no heap allocation on the intended success path.
- `ij_logger_log` must perform no sink I/O.
- `ij_logger_log` must perform no producer-side blocking waits.
- `ij_logger_log` must copy at most the validated RC event payload defined in `docs/api/EVENT_MODEL.md`.

## Bounded Copy Rules

- One event must not exceed 4096 validated textual bytes before queue publication.
- One `message` field must not exceed 2048 bytes.
- One attribute key must not exceed 64 bytes.
- One string attribute value must not exceed 512 bytes.
- One event must not exceed 32 attributes.

These are contract limits, not tuning hints.

## Layout Rules

- Producer reservation metadata, consumer progress metadata, and drop counters must not share one cache line.
- Ring-slot metadata must be aligned to reduce false sharing on a 64-byte cacheline assumption.
- Event payload storage must be fixed-size per slot for the RC.
- The RC runtime must not rely on pointer chasing into caller-owned buffers after queue publication.

## Excluded Producer-Path Work

- local-time conversion
- JSON or syslog wire encoding
- socket send or file write
- retry loops with unbounded iteration
- dynamic buffer growth

## Non-Goals

- SIMD acceleration requirements are not part of the RC.
- NUMA-aware queue partitioning is not part of the RC.
- Compression or encryption on the producer path is not part of the RC.
