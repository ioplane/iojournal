# RC Backpressure Policy For v0.1.0-rc.1

[![C23](https://img.shields.io/badge/C-23-blue)](https://en.cppreference.com/w/c/23)
[![SemVer](https://img.shields.io/badge/SemVer-2.0.0-green)](https://semver.org/spec/v2.0.0.html)
[![OWASP Logging](https://img.shields.io/badge/OWASP-Logging%20Cheat%20Sheet-red)](https://cheatsheetseries.owasp.org/cheatsheets/Logging_Cheat_Sheet.html)

## Scope

- Freeze the RC overflow policy for the bounded MPSC ring.
- Exclude blocking and destructive dequeue variants from the producer hot path.
- Define minimum observability obligations for dropped events.

## Primary Overflow Policy

The RC overflow policy is `DROP_NEW`.

When the ring has no free slot for the current producer:

- the new event is rejected
- the queue contents already accepted into the ring remain intact
- `ij_logger_log` returns `IJ_STATUS_QUEUE_FULL`
- the overflow outcome is the same regardless of later sink type
- overflow must not produce partial JSON, NDJSON, or syslog records

## Excluded RC Policies

- `DROP_OLD` is excluded from the RC because it discards events that already passed validation and publication ordering.
- Producer-side blocking is excluded from the RC hot path.
- Unbounded dynamic growth is excluded from the RC.

## Metric Obligations

The runtime must maintain bounded counters or gauges for:

- total dropped events
- last overflow reason
- queue high-water mark
- successful enqueue count

These metrics may remain internal in the RC, but the implementation must keep them available for tests and diagnostics.

## Flush And Shutdown Rules

- `ij_logger_flush` may wait for the consumer outside the normal hot path.
- Queue overflow during normal logging must not trigger an implicit flush or blocking retry.
- Shutdown must not silently resurrect events previously dropped under `DROP_NEW`.
- `ERROR` and `FATAL` events follow the same bounded overflow policy in the RC; no emergency bypass path is defined yet.

## Non-Goals

- Adaptive backpressure policies are not part of the RC.
- Sink-specific overflow policies are not part of the RC.
- Producer sleep or backoff protocols are not part of the RC contract.
