# Canonical Standards List For v0.1.0-rc.1

## Scope Classes

| Class | Meaning |
|---|---|
| RFC | published IETF specification with local text mirror |
| Guidance | non-RFC reference used for policy, naming, or interoperability boundaries |

## RC Target Set

| Reference | Class | RC Status | Contract Surface | RC Boundary | First Artifact |
|---|---|---|---|---|---|
| RFC 5424 | RFC | MUST | syslog message structure, PRI semantics, header fields, structured data | implementation target for `v0.1.0-rc.1` | Sprint 02 corpus and Sprint 07 syslog formatter/sink docs |
| RFC 5426 | RFC | MUST | syslog UDP transport contract | implementation target for `v0.1.0-rc.1` | Sprint 02 corpus and Sprint 07 UDP sink docs |
| RFC 6587 | RFC | MUST | syslog TCP framing contract, especially octet counting | implementation target for `v0.1.0-rc.1` | Sprint 02 corpus and Sprint 07 TCP sink docs |
| RFC 3339 | RFC | MUST | event timestamp profile | implementation target for `v0.1.0-rc.1` | Sprint 02 corpus and Sprint 03 API/data model docs |
| RFC 8259 | RFC | MUST | JSON syntax and encoder validity | implementation target for `v0.1.0-rc.1` | Sprint 02 corpus and Sprint 03/06 encoder docs |
| NDJSON conventions | Guidance | MUST | append-only file sink line format | implementation target for `v0.1.0-rc.1` | Sprint 02 corpus and Sprint 06 file sink docs |
| OWASP Logging Cheat Sheet | Guidance | MUST | exclusion rules, redaction defaults, sensitive field handling | implementation target for `v0.1.0-rc.1` | Sprint 02 corpus and Sprint 03/05 redaction docs |
| RFC 3164 | RFC | SHOULD | legacy syslog comparison surface | comparison-only, not an RC implementation target | Sprint 02 divergence notes |
| Elastic Common Schema | Guidance | SHOULD | field naming alignment and interoperability guidance | names may be borrowed where they do not distort the core API | Sprint 02 field naming policy |
| OpenTelemetry log model | Guidance | SHOULD | trace/log field alignment and future exporter compatibility | no exporter or protocol implementation in RC | Sprint 02 field naming policy |
| RFC 5425 | RFC | DEFERRED | syslog over TLS transport | excluded from `v0.1.0-rc.1`; mirror retained for defer rationale | backlog after Sprint 08 |

## Frozen Non-Goals

- OTLP protocol details are post-RC.
- Splunk HEC is post-RC.
- Elastic Bulk protocol details are post-RC.
- RFC 3164 compatibility does not expand RC scope beyond comparison and downgrade notes.
