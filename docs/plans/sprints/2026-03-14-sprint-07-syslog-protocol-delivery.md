# Sprint 07: Syslog Protocol Delivery Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** implement the RC syslog feature surface: RFC 5424 formatting plus RFC 5426 UDP and RFC 6587 TCP sinks.

**Architecture:** keep formatting and transport boundaries clean. Do not include TLS in this sprint.

**Tech Stack:** C23, syslog formatter, UDP/TCP sockets, Unity tests, golden fixtures.

---

### Task 1: Implement RFC 5424 formatting

**Files:**
- Create: `src/encoders/ij_rfc5424.c`
- Create: `include/iojournal/iojournal_syslog.h`
- Create: `tests/unit/test_rfc5424.c`
- Create: `tests/fixtures/rfc5424/*.golden`

- [x] **Step 1: Write the failing formatter tests**

Run: targeted RFC 5424 formatter tests
Expected: fail before formatter implementation exists.

- [x] **Step 2: Implement the formatter**

Expected: severity, facility, header fields, structured data, and message formatting follow the RC contract.

- [x] **Step 3: Verify formatter tests**

Run: `ctest --preset clang-debug --output-on-failure -R rfc5424`
Expected: RFC 5424 tests pass.

### Task 2: Implement UDP and TCP syslog sinks

**Files:**
- Create: `src/sinks/ij_syslog_udp.c`
- Create: `src/sinks/ij_syslog_tcp.c`
- Create: `tests/unit/test_syslog_udp.c`
- Create: `tests/unit/test_syslog_tcp.c`

- [x] **Step 1: Write the failing transport tests**

Run: targeted UDP/TCP syslog tests
Expected: fail before transport sinks exist.

- [x] **Step 2: Implement RFC 5426 and RFC 6587 delivery**

Expected: UDP datagram behavior and TCP octet-counting framing are implemented.

- [x] **Step 3: Verify transport tests**

Run: `ctest --preset clang-debug --output-on-failure -R syslog`
Expected: syslog transport tests pass.

### Task 3: Add interoperability evidence and docs

**Files:**
- Create: `docs/en/02-syslog-contract.md`
- Create: `docs/ru/02-syslog-contract.md`
- Create: `examples/syslog_udp.c`
- Create: `examples/syslog_tcp.c`

- [x] **Step 1: Add protocol docs**

Expected: docs explain RFC 5424/5426/6587 surface and RC exclusions.

- [x] **Step 2: Add examples and fixtures**

Expected: examples and golden outputs demonstrate valid messages and framing.

- [x] **Step 3: Verify RC feature surface**

Run: full quality gate plus targeted syslog tests
Expected: repository checks pass and syslog surface is ready for RC hardening.
