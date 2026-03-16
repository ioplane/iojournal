# iojournal Structural Hot-Path Redesign

**Date:** `2026-03-16`

## Goal

Reduce the remaining RC hot-path cost in `iojournal` without changing the public API, sink surface,
redaction semantics, RFC behavior, or synchronous contract.

## Problem

The current active profiler source of record shows that the hottest enabled rows are still dominated by:

- `ij_json_console_encode`
- `ij_event_copy_from_input`
- `ij_json_escape_find_first_special`
- `ij_redact_owned_attr_value`
- `ij_key_should_redact`

The queue, sink writes, and `io_uring` are no longer the main optimization frontier for the RC line.

## Chosen Approach

Use a structural hot-path redesign rather than an architectural behavior change.

This redesign keeps:

- synchronous public API
- existing event, redaction, and RFC semantics
- existing sink families and release scope

This redesign changes only internal hot-path organization:

1. fuse validation, sizing, copy, and redaction preparation into a tighter event-copy pipeline
2. make the JSON and NDJSON builders length-aware so they reuse already-known string lengths
3. reduce `snprintf` and generic formatting overhead where fixed-format output is available
4. replace the linear redaction-key matcher with a cheaper dispatch structure

## Non-Goals

- no async worker semantics
- no new sinks
- no public ABI changes
- no `io_uring` reopening for the RC line
- no `AVX-512`, `SVE`, or `SVE2`

## Acceptance Criteria

- all existing unit and analyzer gates remain green
- output semantics stay unchanged for JSON, NDJSON, and syslog paths
- the shared local rows `enabled_console`, `medium_message_with_metadata`, `contention_mpsc`, and
  `append_file` all improve versus the current source of record
- release-facing profiler evidence is refreshed after the redesign tasks land

## Execution Order

1. fused event validation and copy pipeline
2. length-aware JSON and NDJSON builder path
3. timestamp and scalar formatting cleanup
4. redaction matcher rewrite
5. benchmark, profiler, and comparison refresh
