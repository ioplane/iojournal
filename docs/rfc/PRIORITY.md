# RFC Priority Map For v0.1.0-rc.1

## MUST

| Reference | Contract Surface | First Decision Sprint | RC Obligation |
|---|---|---|---|
| RFC 5424 | syslog message formatting, PRI semantics, version/header fields, structured data rules | 02 | formatter and mapping rules must be frozen before Sprint 07 |
| RFC 5426 | UDP transport for syslog delivery | 02 | transport contract must be frozen before Sprint 07 |
| RFC 6587 | TCP framing, especially octet counting | 02 | framing contract must be frozen before Sprint 07 |
| RFC 3339 | timestamp profile for event serialization | 02 | timestamp format must be fixed before Sprint 03 API modeling |
| RFC 8259 | JSON syntax and encoder validity | 02 | JSON output must stay conformant for console and file sinks |
| NDJSON conventions | newline-delimited file sink output | 02 | file sink must emit one valid JSON object per line |
| OWASP Logging Cheat Sheet | default exclusions, redaction inputs, sensitive field handling | 02 | redaction policy must exist before Sprint 03 and Sprint 05 |

## SHOULD

| Reference | Contract Surface | First Decision Sprint | RC Obligation |
|---|---|---|---|
| Elastic Common Schema | field naming guidance and interoperability boundaries | 02 | adopt only where names do not distort the core API |
| OpenTelemetry log model | trace/log field alignment for future exporter work | 02 | preserve forward-compatibility, no exporter in RC |
| RFC 3164 | legacy syslog comparison and compatibility notes only | 02 | comparison input, not an implementation target |

## DEFERRED

| Reference | Deferred Boundary | Re-entry Point |
|---|---|---|
| RFC 5425 | syslog over TLS is outside `v0.1.0-rc.1` | backlog after Sprint 08 |
| OTLP protocol details | exporter work starts after the first public RC | backlog after Sprint 08 |
| Splunk HEC specifics | vendor delivery protocol is post-RC | backlog after Sprint 08 |
| Elastic Bulk specifics | vendor delivery protocol is post-RC | backlog after Sprint 08 |

## Sprint Mapping

- Sprint 01: source catalog, local mirror boundary, priority map
- Sprint 02: canonical corpus, protocol contracts, field naming and timestamp decisions
- Sprint 03: API and event model constrained by RFC 3339, RFC 8259, and OWASP logging policy
- Sprint 06: file sink constrained by NDJSON conventions and JSON validity
- Sprint 07: implementation of RFC 5424, RFC 5426, and RFC 6587

## Non-Goals Locked By This Map

- No OTLP exporter in `v0.1.0-rc.1`.
- No syslog over TLS in `v0.1.0-rc.1`.
- No vendor-specific delivery protocols in `v0.1.0-rc.1`.
