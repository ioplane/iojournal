# RC Queue Model For v0.1.0-rc.1

[![C23](https://img.shields.io/badge/C-23-blue)](https://en.cppreference.com/w/c/23)
[![RFC 5424](https://img.shields.io/badge/RFC-5424-syslog-orange)](https://www.rfc-editor.org/rfc/rfc5424.html)
[![SemVer](https://img.shields.io/badge/SemVer-2.0.0-green)](https://semver.org/spec/v2.0.0.html)

## Scope

- Freeze the primary queue topology for the RC runtime.
- Define capacity rules and ownership transitions before implementation begins.
- Defer alternate queue families that would widen the RC contract.

## Primary Queue Decision

The RC runtime uses one bounded MPSC ring:

- multiple producer threads call `ij_logger_log`
- one consumer thread owns dequeue and sink delivery
- producers publish immutable event copies into fixed ring slots

This is the only required queue topology for `v0.1.0-rc.1`.

## Decision Rationale

- The public API and examples already imply concurrent producers.
- A single consumer preserves deterministic sink ordering once events are published.
- One bounded ring keeps overflow semantics uniform across console, file, and syslog sinks.
- SPSC-only topology would require external serialization by callers and would undercut the RC logging contract.

## Capacity Rules

| Rule | Requirement |
| --- | --- |
| Ring shape | power-of-two slot count |
| Minimum capacity | 256 slots |
| Default capacity | 1024 slots |
| Maximum RC capacity | 65536 slots |
| Slot ownership | producer-owned until publish, consumer-owned after acquire |

## Ordering Rules

- Publication order is defined by successful enqueue completion, not by wall-clock timestamp.
- The consumer observes one total dequeue order from the shared ring.
- Per-producer relative order must be preserved.
- Cross-producer order is defined by queue publication order and must remain stable for one execution.

## Deferred Alternatives

- SPSC runtime mode as the primary RC topology is deferred.
- MPMC topology is deferred.
- Per-sink dedicated producer queues are deferred.
- Shared-memory multi-process queues are deferred.

## Cross-Document Dependencies

- Overflow behavior is frozen in [`BACKPRESSURE_POLICY.md`](BACKPRESSURE_POLICY.md).
- Atomic visibility and slot reuse rules are frozen in [`MEMORY_ORDERING.md`](MEMORY_ORDERING.md).
- Hot-path constraints are frozen in [`HOT_PATH_LIMITS.md`](HOT_PATH_LIMITS.md).
