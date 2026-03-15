# Standards Feature Matrix For v0.1.0-rc.1

## Matrix

| Reference | Feature Surface | RC Status | First Decision Sprint | First Implementation Sprint | First Artifact |
|---|---|---|---|---|---|
| RFC 5424 | syslog formatter, PRI calculation, version/header layout, structured data rules | MUST | 02 | 07 | `docs/rfc/SYSLOG_CONTRACT.md` and Sprint 07 implementation docs |
| RFC 5426 | UDP delivery for syslog messages | MUST | 02 | 07 | `docs/rfc/SYSLOG_CONTRACT.md` and Sprint 07 UDP sink docs |
| RFC 6587 | TCP framing for syslog messages | MUST | 02 | 07 | `docs/rfc/SYSLOG_CONTRACT.md` and Sprint 07 TCP sink docs |
| RFC 3339 | event timestamps, serialization format, timezone normalization | MUST | 02 | 03 | `docs/rfc/TIMESTAMP_POLICY.md` and Sprint 03 API/data model docs |
| RFC 8259 | JSON encoder validity for console and file sinks | MUST | 02 | 03 | `docs/rfc/JSON_NDJSON_CONTRACT.md` and Sprint 03/06 encoder docs |
| NDJSON conventions | one JSON object per line in append-only file output | MUST | 02 | 06 | `docs/rfc/JSON_NDJSON_CONTRACT.md` and Sprint 06 file sink docs |
| OWASP Logging Cheat Sheet | redaction defaults, exclusion list, sensitive field masking | MUST | 02 | 03 | `docs/rfc/REDACTION_POLICY.md` and Sprint 03/05 policy docs |
| Elastic Common Schema | field naming alignment where neutral to core API design | SHOULD | 02 | 03 | `docs/rfc/FIELD_NAMING_POLICY.md` |
| OpenTelemetry log model | trace and span field naming compatibility | SHOULD | 02 | 03 | `docs/rfc/FIELD_NAMING_POLICY.md` |
| RFC 3164 | comparison matrix for legacy syslog behavior | SHOULD | 02 | 07 | `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` |
| RFC 5425 | syslog over TLS transport | DEFERRED | 02 | post-RC | `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` and `docs/plans/BACKLOG.md` |
| OTLP protocol details | exporter wire protocol | DEFERRED | 02 | post-RC | `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` and `docs/plans/BACKLOG.md` |
| Splunk HEC specifics | vendor HTTP ingestion protocol | DEFERRED | 02 | post-RC | `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` and `docs/plans/BACKLOG.md` |
| Elastic Bulk specifics | vendor bulk ingestion protocol | DEFERRED | 02 | post-RC | `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` and `docs/plans/BACKLOG.md` |

## Consistency Rules

- Every `MUST` item must have a Sprint 02 decision artifact before implementation starts.
- No `DEFERRED` item may enter `v0.1.0-rc.1` implementation scope without a roadmap change.
- `SHOULD` items may influence naming and comparison notes, but they do not expand the RC implementation target set.
