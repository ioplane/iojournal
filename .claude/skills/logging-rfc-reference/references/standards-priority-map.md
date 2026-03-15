# Logging Standards Priority Map

## Formatter And Transport

1. RFC 5424
   - syslog message format
   - severity and facility mapping
   - structured data rules
2. RFC 5426
   - one syslog message per UDP datagram
   - payload sizing constraints
3. RFC 6587
   - TCP framing
   - prefer octet counting
4. RFC 5425
   - TLS transport mapping for syslog

## Structured Logging And Export

1. OpenTelemetry log model
   - severity number mapping
   - attribute structure
   - OTLP payload fields
2. OTLP/HTTP
   - `/v1/logs`
   - retry and payload expectations
3. ECS
   - dotted field names for interoperability

## Formats And Security

1. RFC 3339 / ISO 8601
   - timestamps
2. NDJSON
   - one JSON object per line
   - final newline for bulk-oriented streams
3. OWASP logging guidance
   - never log raw secrets, tokens, passwords, or credentials
   - preserve safe redaction defaults
