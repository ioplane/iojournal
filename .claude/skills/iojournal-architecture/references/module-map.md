# iojournal Module Map

## Layer Ownership

- `core`
  - runtime context
  - ring buffers and queues
  - worker handoff and bounded memory ownership
- `filters`
  - level gating
  - sampling and rate limiting
  - redaction and denylist logic
- `encoders`
  - JSON
  - RFC 5424 syslog
  - OTLP and NDJSON shaping
- `sinks`
  - console and file delivery
  - syslog UDP/TCP/TLS
  - OTLP, Splunk HEC, Elastic bulk exporters

## Invariants

- Producers write into preallocated storage only.
- Formatting should move off the producer path when it would increase latency variance.
- Overflow behavior must be explicit:
  - `DROP_NEW`
  - `DROP_OLD`
  - `BLOCK`
  - `SAMPLE`
- Secret-bearing attributes stay redactable across every encoder and sink.

## Boundary Checks

Before changing architecture:
1. Does the change belong in `core`, `filters`, `encoders`, or `sinks`?
2. Does it change hot-path memory behavior?
3. Does it blur ownership with `iohttp`, `liboas`, or tracing consumers?
4. Does it require new redaction or backpressure policy?
