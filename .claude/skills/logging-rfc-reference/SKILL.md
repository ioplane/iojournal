---
name: logging-rfc-reference
description: Use when work depends on syslog framing, OTLP log export, ECS field names, NDJSON output, timestamp formats, or logging security/redaction rules. Mandatory for encoders, export sinks, log schema changes, and standards-sensitive formatting work in iojournal.
---

# Logging Rfc Reference

## Overview

Use this skill when a change depends on external logging standards rather than local implementation preference. Start with the priority map before changing formatter output, transport framing, or security-sensitive field handling.

## Priority Order

- RFC 5424 for syslog message structure and severity/facility mapping.
- RFC 5426 for UDP transport limits.
- RFC 6587 for TCP framing, preferring octet counting.
- RFC 5425 for TLS transport mapping.
- OpenTelemetry log model and OTLP when changing log export payloads.
- ECS for structured field names when interoperability matters.
- RFC 3339 / ISO 8601 for timestamps.
- NDJSON rules for bulk and line-oriented JSON export.
- OWASP logging guidance for redaction, secret handling, and unsafe fields.

## Workflow

1. Identify the output surface:
   - formatter
   - transport
   - schema
   - security rule
2. Read the matching reference section before changing code or docs.
3. Keep deliberate divergence explicit in code comments, tests, or architecture notes.

## Repository Position

- Prefer standards-aligned field names and transport behavior.
- Default to safe redaction for secrets, tokens, cookies, and credentials.
- Do not trade standards compliance for convenience in public-facing encoders.

## References

- `references/standards-priority-map.md`
