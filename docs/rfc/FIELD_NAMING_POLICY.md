# Field Naming Policy For v0.1.0-rc.1

## Governing References

- Elastic Common Schema
- OpenTelemetry log data model
- RFC 5424

## Scope

This document freezes the naming contract for the library-native event payload used by:

- JSON and NDJSON sinks
- future RFC 5424 projection rules
- future post-RC exporter mappings

## Naming Rules

| Rule | Requirement |
|---|---|
| Native key style | lowercase `snake_case` |
| Reserved key space | top-level keys listed in this document |
| User metadata container | `attributes` object |
| ECS alignment | guidance only, not full schema adoption |
| OpenTelemetry alignment | guidance only, not full model adoption |
| Dotted keys | excluded from the native RC payload |

## Reserved Top-Level Keys

| Key | Meaning | Status |
|---|---|---|
| `timestamp` | event time in canonical RFC 3339 form | required |
| `level` | textual severity label | required |
| `message` | human-readable event message | required |
| `logger` | logger or subsystem identifier | optional |
| `facility` | syslog facility identifier when relevant | optional |
| `severity_number` | normalized numeric severity for downstream mapping | optional |
| `trace_id` | distributed trace identifier | optional |
| `span_id` | distributed span identifier | optional |
| `attributes` | structured user metadata object | optional |

## User Attribute Rules

- User-defined metadata must live under `attributes`.
- User-defined keys must not overwrite reserved top-level keys.
- User-defined keys inside `attributes` should also use lowercase `snake_case` where the caller controls the schema.
- The RC core does not rewrite arbitrary caller keys into ECS dotted paths.

## Interoperability Rules

- `trace_id` and `span_id` are reserved to preserve OpenTelemetry-compatible naming.
- ECS-style dotted names such as `host.name` and `event.category` are not part of the native RC payload contract.
- Exporters added after RC may derive alternate schemas from the native payload, but the native payload remains snake_case.

## RFC 5424 Projection Boundary

- Native JSON keys and RFC 5424 field names are separate contracts.
- `facility` and `severity_number` may be projected into PRI calculation when the syslog sink is used.
- `attributes` are not automatically guaranteed to become RFC 5424 structured data in RC unless the syslog contract allows that projection.

## Non-Goals

- Full ECS schema adoption is not in RC scope.
- Full OpenTelemetry log model adoption is not in RC scope.
- Automatic renaming of arbitrary user keys is not in RC scope.
