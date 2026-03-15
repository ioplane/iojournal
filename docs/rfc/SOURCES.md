# RFC And Standards Sources

## Canonical Source Set

| Source | Role | Authority Boundary | Local Artifact |
|---|---|---|---|
| RFC Editor | canonical final RFC text and publication metadata | final text for published RFCs | `docs/rfc/rfc*.txt` |
| IETF Datatracker API | searchable metadata for RFCs and unexpired Internet-Drafts | discovery, metadata, and draft recency | `docs/rfc/registry.md` |
| OpenTelemetry specification | future log model and trace-field alignment | reference guidance only, not RC scope | Sprint 02 field naming notes |
| Elastic Common Schema | field naming and interoperability guidance | reference guidance only, not full schema adoption | Sprint 02 field naming notes |
| OWASP Logging Cheat Sheet | security and exclusion baseline for logging payloads | policy input for redaction defaults | Sprint 02 redaction decisions |

## Retrieval Contract

- Use RFC Editor as the source of truth for published RFC text.
- Use IETF Datatracker for search, metadata refresh, and active draft discovery.
- Mirror only references that are either in RC scope or needed for explicit defer decisions.
- Store final decisions in repository docs. External sites are inputs, not the long-term decision store.
- Keep the Sprint 01 mirror minimal and expand it only when a later sprint consumes the reference directly.

## RC Target Set

| Reference | RC Status | Why It Is In Scope | First Consuming Sprint | Local Mirror Expectation |
|---|---|---|---|---|
| RFC 5424 | MUST | syslog message format, PRI semantics, structured data | 02 | mirror text |
| RFC 5426 | MUST | UDP transport contract for syslog delivery | 02 | mirror text |
| RFC 6587 | MUST | TCP framing contract for syslog delivery | 02 | mirror text |
| RFC 3339 | MUST | timestamp profile for event serialization | 02 | mirror text |
| RFC 8259 | MUST | JSON syntax for console and file encoders | 02 | mirror text |
| NDJSON conventions | MUST | append-only file sink line format | 02 | manual repo guidance |
| OWASP Logging Cheat Sheet | MUST | exclusion and redaction baseline | 02 | manual repo guidance |
| RFC 3164 | SHOULD | legacy syslog comparison and downgrade boundary only | 02 | mirror text |
| Elastic Common Schema | SHOULD | field naming alignment, not full schema adoption | 02 | docs reference only |
| OpenTelemetry log model | SHOULD | trace/log field alignment for post-RC exporters | 02 | docs reference only |
| RFC 5425 | DEFERRED | TLS syslog transport is outside `v0.1.0-rc.1` | 02 | mirror text retained for defer rationale |

## Local Mirror Policy

- The default Sprint 01 mirror consists of `RFC 3164`, `RFC 3339`, `RFC 5424`, `RFC 5425`, `RFC 5426`, `RFC 6587`, and `RFC 8259`.
- Add new RFC text files only when the roadmap or priority map introduces a concrete consumer.
- Do not mirror vendor-specific protocol references during RC planning.

## Scraper Workflow

The Python corpus workflow is:

```bash
python3 scripts/rfc-scraper.py -o docs/rfc/registry.md
python3 scripts/rfc-scraper.py --download docs/rfc
```

- The generated registry is the discovery surface.
- The `.txt` mirror is the implementation reading surface.
- Use `--download-all` only for deliberate corpus expansion beyond the Sprint 01 mirror set.

## Explicit Non-RC References

- OTLP wire-level references are post-RC.
- Splunk HEC and Elastic Bulk protocol references are post-RC.
- TLS-specific syslog implementation work stays deferred even though `RFC 5425` is mirrored.
