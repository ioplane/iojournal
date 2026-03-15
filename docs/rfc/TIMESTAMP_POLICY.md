# Timestamp Policy For v0.1.0-rc.1

## Governing References

- RFC 3339
- RFC 5424

## Scope

This document defines the timestamp contract for:

- the public event model planned for Sprint 03
- JSON and NDJSON encoders planned for Sprint 03 and Sprint 06
- RFC 5424 syslog formatting planned for Sprint 07

## Output Contract

| Rule | Requirement |
|---|---|
| Format | RFC 3339 timestamp string |
| Timezone | UTC only for library-produced output |
| UTC marker | `Z` suffix only |
| Fractional seconds | exactly 3 digits for `v0.1.0-rc.1` |
| Minimum precision | millisecond |
| Date separator | `-` |
| Time separator | `:` |
| Date/time boundary | literal `T` |

## Canonical Form

The canonical serialized form is:

```text
YYYY-MM-DDTHH:MM:SS.sssZ
```

Example:

```text
2026-03-15T00:42:13.127Z
```

## Required Behavior

- Every emitted event must carry a timestamp.
- Library-produced timestamps must be normalized to UTC before formatting.
- Library-produced timestamps must use four-digit year, two-digit month, two-digit day, two-digit hour, two-digit minute, and two-digit second fields.
- Fractional seconds must always be present in RC output, even when the millisecond component is `000`.
- Leap-second textual output is out of scope for RC; the serializer must not emit `:60`.
- Sink-specific serializers must not emit local offsets such as `+03:00` in RC output.

## Internal Representation Boundary

- Internal storage format is not frozen by this document.
- Implementations may store timestamps as Unix epoch plus subsecond component.
- Internal precision may exceed milliseconds.
- External RC serialization must still normalize to the canonical form above.

## Non-Goals

- Parsing arbitrary timestamp strings is not in RC scope.
- Sink-specific local-time rendering is not in RC scope.
- Variable-width fractional precision is not in RC scope.
