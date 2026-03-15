# JSON And NDJSON Contract For v0.1.0-rc.1

## Governing References

- RFC 8259
- NDJSON conventions
- RFC 3339

## Scope

This document defines the contract for:

- JSON console output
- JSON file sink output
- NDJSON append-only file output

## JSON Rules

| Rule | Requirement |
|---|---|
| Syntax | every emitted object must be valid RFC 8259 JSON |
| Encoding | UTF-8 only |
| Top-level value | one JSON object per event |
| Non-finite numbers | forbidden |
| Duplicate keys | forbidden in library-produced payloads |
| Canonical timestamp field | `timestamp` per `TIMESTAMP_POLICY.md` |

## Required Top-Level Fields

| Key | Requirement |
|---|---|
| `timestamp` | required |
| `level` | required |
| `message` | required |

All other reserved fields are optional and governed by `FIELD_NAMING_POLICY.md`.

## Determinism Rules

- Library-produced payloads must use a stable field order for reserved top-level keys.
- When `attributes` are present, `attributes` must appear after reserved top-level keys.
- The stable order requirement exists for fixture generation and regression testing; JSON consumers must still treat object ordering as non-semantic.

## NDJSON Rules

| Rule | Requirement |
|---|---|
| Record shape | one valid JSON object per line |
| Separator | `LF` (`\n`) |
| BOM | forbidden |
| Array wrapper | forbidden |
| Trailing commas | forbidden |
| Empty line records | forbidden |

## File Sink Contract

- The RC file sink must emit append-only NDJSON.
- Every completed line must be independently parseable as JSON.
- Multi-line pretty-printing is not allowed in RC file output.
- If an event cannot be serialized as valid JSON, it must not be partially written as an NDJSON line.

## String And Character Rules

- Strings must be JSON-escaped before emission.
- Embedded newlines inside `message` or `attributes` values must remain escaped within the JSON string representation.
- Binary payloads are not part of the native RC JSON contract.

## Non-Goals

- Alternate JSON schema profiles are not in RC scope.
- Pretty-printed file output is not in RC scope.
- JSON arrays as batch output are not in RC scope.
