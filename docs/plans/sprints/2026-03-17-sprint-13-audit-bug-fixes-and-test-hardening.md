# Sprint 13: Audit Bug Fixes And Test Hardening Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix all critical and high-severity bugs found in the independent code audit (2026-03-17) and close the critical test coverage gaps that could hide real production bugs.

**Architecture:** All fixes are internal — no public API or ABI changes. Bug fixes come first (Tasks 1–6), then missing tests (Tasks 7–11). Each task is independently committable.

**Tech Stack:** C23, Unity test framework, CMake, Podman dev container.

**Status:** COMPLETE. All 11 tasks done. quality.sh PASS 14/14, ctest 17/17.

---

### Task 1: Fix UTF-8 scalar out-of-bounds read — DONE

File: `src/core/ij_utf8_scalar.c`
Fix: moved `remaining` bounds check before continuation byte reads for 3-byte and 4-byte sequences.

### Task 2: Fix redaction buffer overflow guard — DONE

Files: `src/filters/ij_redact.c`, `src/ij_internal.h`, `tests/unit/test_filters.c`
Fix: added `size_t value_capacity` parameter to `ij_redact_owned_attr_value_n` with early-return guard.

### Task 3: Fix SIMD dispatch for ARM NEON — DONE

File: `src/encoders/ij_simd_dispatch.h`
Fix: added `ij_simd_runtime_has_arm_neon()` checks to `ij_simd_json_escape_kind()` and `ij_simd_utf8_kind()`.

### Task 4: Fix RFC 5424 APP-NAME sanitization — DONE

File: `src/encoders/ij_rfc5424.c`
Fix: added `ij_syslog_sanitize_app_name()` that replaces non-printable/space chars with `_`, clamps to 48 bytes.

### Task 5: Fix file sink clock step-back — DONE

File: `src/sinks/ij_file_sink.c`
Fix: added `now > sink->last_rotation_epoch` guard before uint64_t cast in rotation check.

### Task 6: Fix logger config deep copy — DONE

File: `src/core/ij_logger.c`
Fix: `strdup` for `logger_name` and `syslog_host` in `ij_apply_logger_defaults`, freed in `ij_logger_shutdown`.

### Task 7: Add ring queue concurrency tests — DONE

File: `tests/unit/test_queue.c`
Added: 3 tests (empty dequeue, MPSC 4-thread stress with 1000 events, producer-consumer with atomic done flag).

### Task 8: Add UTF-8 validation unit tests — DONE

File: `tests/unit/test_utf8.c` (new), `tests/unit/CMakeLists.txt`
Added: 15 tests (valid ASCII/2/3/4-byte, mixed, invalid bare continuation/0xff/overlong/surrogate/above-max, truncated 2/3/4-byte, empty string, dispatch-vs-scalar).

### Task 9: Add SIMD scalar-vs-AVX2 equivalence tests — DONE

File: `tests/unit/test_simd_dispatch.c`
Added: 5 tests (safe ASCII, special at boundaries, control chars, valid long string, invalid at position 33).

### Task 10: Add encoder buffer overflow tests — DONE

Files: `tests/unit/test_console_json.c`, `tests/unit/test_rfc5424.c`
Added: 4 tests (tiny buffer, zero buffer, RFC 5424 tiny buffer, APP-NAME sanitization verification).

### Task 11: Add event validation negative tests — DONE

File: `tests/unit/test_event.c`
Added: 6 tests (NaN double, Infinity double, NULL string data, oversized string, unknown kind, excess count).

### Sprint 13 Exit Criteria — MET

- [x] All 6 bugs fixed
- [x] ctest 17/17 pass (was 16/16)
- [x] quality.sh PASS: 14, FAIL: 0 inside Podman
- [x] No public API or ABI changes
