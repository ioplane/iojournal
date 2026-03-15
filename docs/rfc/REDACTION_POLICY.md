# Redaction Policy For v0.1.0-rc.1

## Governing References

- OWASP Logging Cheat Sheet

## Scope

This document freezes the minimum redaction behavior for:

- structured event attributes
- sink output formatting
- future API defaults introduced in Sprint 03 and Sprint 05

## Core Rules

| Rule | Requirement |
|---|---|
| Default mode | redaction on by default |
| Application point | before sink-specific formatting |
| Structured metadata | must be checked against the denylist |
| Replacement token | `[REDACTED]` |
| Key matching | case-insensitive |

## Default Denylist Categories

| Category | Representative Keys |
|---|---|
| Passwords | `password`, `passwd`, `passphrase` |
| Secrets | `secret`, `client_secret` |
| Tokens | `token`, `access_token`, `refresh_token`, `id_token` |
| API credentials | `api_key`, `apikey`, `authorization` |
| Session identifiers | `session_id`, `session`, `cookie`, `set_cookie` |
| Cryptographic material | `private_key`, `secret_key`, `signing_key` |

## Required Behavior

- If a structured field key matches the denylist, the emitted value must be replaced with `[REDACTED]`.
- Redaction must happen before JSON serialization and before RFC 5424 formatting.
- Redaction must preserve the key so downstream consumers can see that data was intentionally withheld.
- Library-provided examples and fixtures must not contain live secrets.

## Message Body Boundary

- The RC default redaction policy applies to structured metadata and named fields.
- Arbitrary free-form message text is not automatically scanned as a full DLP system in RC.
- Applications remain responsible for not formatting raw secrets directly into `message`.

## Transport Boundary

- Redaction is independent of sink type.
- The same redaction result must be visible in console, file, and syslog outputs for the same event payload.

## Non-Goals

- Full PII classification is not in RC scope.
- Format-preserving masking is not in RC scope.
- Deep inspection of arbitrary prose is not in RC scope.
