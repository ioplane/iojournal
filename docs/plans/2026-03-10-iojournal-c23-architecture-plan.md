# iojournal Architecture and Implementation Plan (C23)
**Date:** 2026-03-10

## 1. Executive Summary
`iojournal` is a centralized, high-performance structured logging library for modern Linux ecosystems, built strictly on C23. It serves as the enterprise logging foundation for `iohttp`, `liboas`, and other associated libraries. The primary architectural goals are **zero-allocation hot paths**, **bounded and predictable memory usage**, **lock-free thread concurrency**, and seamless integration with **SIEM/Observability pipelines** (Syslog, OpenTelemetry, Splunk HEC, Elastic Bulk).

## 2. Core Architecture
The pipeline follows a `Producer -> Buffer -> Filter -> Encoder -> Sink` model.

- **Producers (Hot Path):** Application threads dispatch events using macro facades (e.g., `IJ_INFO`). The hot path is bounded and executes with zero heap allocations. String formatting and JSON serialization are deferred.
- **Buffers:** Highly optimized ring buffers. Depending on the concurrency model chosen, these are either `SPSC` (per-thread thread-local buffers syncing to a main bus) or `MPSC` (lock-free bounded queues).
- **Filters:** Middleware applying logic such as severity drops, compile-time omissions, rate limiting, and mandatory **OWASP PII redaction** (hiding passwords, tokens).
- **Encoders:** Convert raw structs to standard formats: RFC 5424 (Syslog), JSON Lines (NDJSON), or Protobuf-like OTLP bodies.
- **Sinks:** Destinations. 
  - *Local:* Console, File (with built-in rotation and retention).
  - *Network (io_uring accelerated):* Syslog over UDP/TCP/TLS, OTLP/HTTP.

## 3. Leverage of Modern C23 Features
`iojournal` extensively utilizes C23 to guarantee type safety, efficiency, and robustness:
- **`constexpr`**: For compile-time buffer limits, log levels, and bitmasks.
- **`typeof` / `auto`**: For type-generic macros and robust container instantiation.
- **`nullptr`**: Replacing ambiguous `NULL` macros in callbacks and pointers.
- **`[[nodiscard]]`, `[[fallthrough]]`, `[[reproducible]]`**: Driving static analysis constraints.
- **`_BitInt(N)`**: Packing metadata fields into cache-line efficient layouts (e.g., 48-bit timestamp + 3-bit level).
- **`__VA_OPT__`**: For trailing comma-free varargs logging macros.
- **`<stdbit.h>` & `<stdckdint.h>`**: Utilizing native bitwise math for power-of-two ring sizing and checked arithmetic for safe bounds computation.

## 4. SIEM & Observability Standards
Integration protocols and RFCs strictly enforced:
- **RFC 5424 / 5426 / 6587**: Syslog formatting, UDP mapping, TCP octet-counting.
- **OpenTelemetry (OTLP)**: Using `/v1/logs` HTTP payloads, matching severity scales (1-24), `TraceId` / `SpanId` injection.
- **Elastic Common Schema (ECS)**: Enforced field naming (e.g., `@timestamp`, `log.level`, `client.ip`).
- **OWASP Guidelines**: Built-in redaction denylist for fields like `authorization`, `x-api-key`, `cookie`.

## 5. io_uring Optimization
For network sinks (Syslog, HTTP exporters) and large file operations:
- Use **io_uring** for asynchronous batch writes to disk.
- Exporters should implement buffer pooling and state-machines tracking the lifecycle to allow `IORING_OP_SEND_ZC` (Zero Copy Send), monitoring `IORING_CQE_F_NOTIF` to safely recycle buffers.
- The logging daemon/flusher thread executes in a separate io_uring event loop to avoid starvation of the host HTTP application.

## 6. Phased Implementation Roadmap
### P0: Core MVP
- Set up C23 `CMakeLists.txt` and project structure.
- Define `ij_event_t` struct, `constexpr` variables, C23 `__VA_OPT__` macros (`IJ_INFO`, etc.).
- Implement lock-free Ring Buffer (`SPSC` or `MPSC`).
- Create basic JSON encoder and stdout/stderr Sink.
- Implement OWASP PII redaction layer.

### P1: Persistence & Legacy SIEM
- Implement asynchronous File Sink with size/time rotation and retention limits.
- Build RFC 5424 formatter.
- Implement Syslog Sinks: UDP (RFC 5426) and TCP (RFC 6587).
- Implement Batching/Flush intervals.

### P2: Modern Observability
- Syslog over TLS (RFC 5425) via wolfSSL.
- OTLP/HTTP (`/v1/logs`) sink implementation.
- Splunk HEC and Elastic NDJSON Bulk sinks.
- io_uring backend integration for sink zero-copy flushing.

### P3: Advanced Enterprise Features
- Disk-backed spooling queues to handle massive network disconnects.
- Multi-process Shared Memory (SHM) bus.
- Encryption-at-rest (AES-GCM) for archived logs.
