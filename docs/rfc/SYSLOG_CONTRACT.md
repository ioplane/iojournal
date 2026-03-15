# Syslog Contract For v0.1.0-rc.1

## Governing References

- RFC 5424
- RFC 5426
- RFC 6587
- RFC 3339

## Scope

This document freezes the RC syslog contract for:

- RFC 5424 message formatting
- RFC 5426 UDP transport
- RFC 6587 TCP framing

Excluded from this document:

- RFC 5425 syslog over TLS
- RFC 3164 emission as a primary RC output mode

## Message Format Contract

RC syslog output must use RFC 5424 message structure:

```text
<PRI>1 TIMESTAMP HOSTNAME APP-NAME PROCID MSGID STRUCTURED-DATA MSG
```

## Field Rules

| Field | Requirement |
|---|---|
| `PRI` | facility × 8 + severity |
| `VERSION` | fixed literal `1` |
| `TIMESTAMP` | must follow `TIMESTAMP_POLICY.md` |
| `HOSTNAME` | emit `-` when unavailable |
| `APP-NAME` | emit `-` when unavailable |
| `PROCID` | emit `-` when unavailable |
| `MSGID` | emit `-` when unavailable |
| `STRUCTURED-DATA` | emit `-` when no valid structured element is available |
| `MSG` | UTF-8 message text |

## Severity And Facility Rules

| Rule | Requirement |
|---|---|
| Severity domain | RFC 5424 severity range `0..7` |
| Default facility | `user` (`1`) unless configured otherwise |
| PRI formula | `(facility * 8) + severity` |

## RC Level Collapse Rules

If the public API later exposes more than the RFC 5424 severity domain, RC syslog emission must collapse internal levels as follows:

| Internal level | RFC 5424 severity |
|---|---|
| `fatal` | `2` |
| `error` | `3` |
| `warn` | `4` |
| `notice` | `5` |
| `info` | `6` |
| `debug` | `7` |
| `trace` | `7` |

## Structured Data Boundary

- `STRUCTURED-DATA` may be `-` in RC when no valid structured data element is available.
- RC does not freeze a library-owned enterprise number or custom SD-ID namespace.
- JSON `attributes` are not automatically guaranteed to appear as RFC 5424 structured data in RC.

## UDP Contract

| Rule | Requirement |
|---|---|
| Governing reference | RFC 5426 |
| Transport unit | exactly one syslog message per datagram |
| Default port | `514/udp` |
| Delivery semantics | best effort only |
| Intentional fragmentation | forbidden |
| Default target limit | 2048 octets |

If a rendered message exceeds the configured UDP size limit, the sink must not split it across datagrams.

## TCP Contract

| Rule | Requirement |
|---|---|
| Governing reference | RFC 6587 |
| Default port | `514/tcp` |
| Framing mode in RC | octet counting only |
| Delimiter framing | excluded from RC |

Octet-counted framing form:

```text
MSG-LEN SP SYSLOG-MSG
```

## Non-Goals

- TLS transport is not in `v0.1.0-rc.1`.
- RFC 3164 output mode is not in `v0.1.0-rc.1`.
- Delimiter-based TCP framing is not in `v0.1.0-rc.1`.
