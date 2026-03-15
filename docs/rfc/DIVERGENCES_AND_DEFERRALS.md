# Divergences And Deferrals For v0.1.0-rc.1

## Purpose

This document records standards-related exclusions and simplifications that are intentional for the first public release candidate.

## Deferred Standards

| Reference | RC Decision | Reason |
|---|---|---|
| RFC 5425 | deferred | TLS transport requires certificate handling, trust policy, and operational verification outside the RC sink set |
| OTLP protocol details | deferred | exporter wire protocol and schema mapping are outside the RC delivery family |
| Splunk HEC specifics | deferred | vendor-specific ingestion contract is outside the RC delivery family |
| Elastic Bulk specifics | deferred | vendor-specific ingestion contract is outside the RC delivery family |

## Comparison-Only Standards

| Reference | RC Decision | Reason |
|---|---|---|
| RFC 3164 | comparison only | RC ships RFC 5424 formatting; RFC 3164 remains a downgrade and interoperability reference only |
| Elastic Common Schema | guidance only | RC preserves native snake_case payloads instead of full ECS adoption |
| OpenTelemetry log model | guidance only | RC preserves future compatibility fields without adding exporter or full semantic model scope |

## Intentional RC Simplifications

| Area | RC Simplification | Reason |
|---|---|---|
| Syslog transport | RFC 6587 octet counting only | delimiter-based framing is legacy behavior outside the first RC |
| Syslog transport security | no RFC 5425 support | keep RC sink surface limited to UDP and TCP |
| Timestamp rendering | UTC with fixed millisecond precision | deterministic fixtures and simpler cross-sink behavior |
| Native field naming | snake_case native payload only | avoid premature ECS or dotted-key lock-in |
| Structured data | no library-owned RFC 5424 SD-ID namespace frozen in RC | enterprise namespace and projection rules need separate contract work |
| JSON file output | NDJSON only | append-only sink behavior is the RC file contract |
| Redaction | key-based default denylist, no full DLP | predictable baseline without claiming full sensitive-data discovery |

## Boundaries Locked By This Document

- No deferred reference may enter `v0.1.0-rc.1` implementation scope without a roadmap change.
- No comparison-only reference may redefine the native event payload for RC.
- Any future exporter-specific schema must be layered on top of the native RC payload, not retroactively redefine it.
