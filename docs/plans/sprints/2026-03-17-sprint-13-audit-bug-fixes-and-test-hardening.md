# Sprint 13: Audit Bug Fixes And Test Hardening Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix all critical and high-severity bugs found in the independent code audit (2026-03-17) and close the critical test coverage gaps that could hide real production bugs.

**Architecture:** All fixes are internal — no public API or ABI changes. Bug fixes come first (Tasks 1–6), then missing tests (Tasks 7–11). Each task is independently committable.

**Tech Stack:** C23, Unity test framework, CMake, Podman dev container.

---

### Task 1: Fix UTF-8 scalar out-of-bounds read

**Files:**
- Modify: `src/core/ij_utf8_scalar.c`
- Test: `tests/unit/test_utf8.c` (new, created in Task 8)

**Bug:** 3-byte and 4-byte continuation bytes are read at lines 31–32 and 45–47 **before** the `remaining < 3U` / `remaining < 4U` bounds checks. If a multi-byte lead byte appears at the end of the buffer, reads are out of bounds.

- [ ] **Step 1: Read the current code**

Read `src/core/ij_utf8_scalar.c` lines 20–60 to understand the validation loop structure.

- [ ] **Step 2: Fix the bounds check ordering**

Move the `remaining` check **before** reading continuation bytes. The fix pattern for the 3-byte case:

```c
/* 3-byte: 0xE0..0xEF */
if (lead >= 0xE0U && lead <= 0xEFU) {
    if (remaining < 3U) {
        return false;
    }
    uint8_t b1 = data[i + 1];
    uint8_t b2 = data[i + 2];
    // ... continuation validation ...
}
```

Same pattern for the 4-byte case: check `remaining < 4U` before reading `data[i+1..3]`.

- [ ] **Step 3: Build and run existing tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass (existing `test_event_copy_rejects_invalid_utf8_message` still passes).

- [ ] **Step 4: Commit**

---

### Task 2: Fix redaction buffer overflow guard

**Files:**
- Modify: `src/filters/ij_redact.c`
- Test: `tests/unit/test_filters.c`

**Bug:** `ij_redact_owned_attr_value_n` writes 11 bytes (`[REDACTED]\0`) into `value->as.string.data` without checking the destination buffer capacity. If the original string is shorter than 10 bytes, this is a heap buffer overflow.

- [ ] **Step 1: Read the current code**

Read `src/filters/ij_redact.c` lines 110–130 to understand the redaction function.

- [ ] **Step 2: Add capacity guard**

Add a capacity parameter or check the existing `len` field before writing:

```c
void ij_redact_owned_attr_value_n(ij_owned_attr_value_t *value, size_t capacity) {
    static const char redacted[] = IJ_REDACTED_LITERAL;
    if (capacity < sizeof(redacted) - 1U) {
        /* Buffer too small — truncate or reject */
        return;
    }
    memcpy(value->as.string.data, redacted, sizeof(redacted));
    value->as.string.len = sizeof(redacted) - 1U;
}
```

If adding a parameter changes the internal signature, update all call sites in `ij_event.c`.

- [ ] **Step 3: Add test for short-string redaction**

In `tests/unit/test_filters.c`, add a test that creates an `ij_owned_attr_value_t` with a 1-byte value and verifies redaction either succeeds (with properly sized arena) or rejects (with undersized buffer).

- [ ] **Step 4: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass including the new test.

- [ ] **Step 5: Commit**

---

### Task 3: Fix SIMD dispatch for ARM NEON

**Files:**
- Modify: `src/encoders/ij_simd_dispatch.h`
- Modify: `tests/unit/test_simd_dispatch.c`

**Bug:** `ij_simd_json_escape_kind()` and `ij_simd_utf8_kind()` only check for AVX2 and fall through to SCALAR. They never check `ij_simd_runtime_has_arm_neon()`, so NEON paths are dead code on aarch64.

- [ ] **Step 1: Read the current dispatch code**

Read `src/encoders/ij_simd_dispatch.h` lines 40–75.

- [ ] **Step 2: Add NEON dispatch branches**

After the AVX2 check, add:

```c
#if defined(__aarch64__) || defined(_M_ARM64)
    if (ij_simd_runtime_has_arm_neon()) {
        return IJ_SIMD_KIND_ARM_NEON;
    }
#endif
```

Apply to both `ij_simd_json_escape_kind()` and `ij_simd_utf8_kind()`.

- [ ] **Step 3: Add dispatch coverage test**

In `test_simd_dispatch.c`, add a test that verifies on non-x86 platforms the dispatch returns a valid non-SCALAR kind (conditional on platform macros).

- [ ] **Step 4: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass. On x86, AVX2 is returned. On aarch64, NEON would be returned.

- [ ] **Step 5: Commit**

---

### Task 4: Fix RFC 5424 APP-NAME sanitization

**Files:**
- Modify: `src/encoders/ij_rfc5424.c`
- Modify: `tests/unit/test_rfc5424.c`

**Bug:** APP-NAME field is not sanitized per RFC 5424. Spaces, newlines, and non-printable characters in `logger_name` pass through to syslog output, enabling log injection.

- [ ] **Step 1: Read the current code**

Read `src/encoders/ij_rfc5424.c` to understand how `app_name` is constructed.

- [ ] **Step 2: Add APP-NAME sanitization**

Write an internal `ij_syslog_sanitize_app_name` function that replaces non-printable and space characters with `_`, and truncates to the RFC 5424 limit (48 bytes). Apply before `snprintf` formatting.

- [ ] **Step 3: Add test for special characters in logger name**

In `test_rfc5424.c`, add a test with `logger_name = "my app\nnewline"` and verify the output contains `my_app_newline` (sanitized) and no raw control characters.

- [ ] **Step 4: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass.

- [ ] **Step 5: Commit**

---

### Task 5: Fix file sink clock step-back spurious rotation

**Files:**
- Modify: `src/sinks/ij_file_sink.c`
- Modify: `tests/unit/test_file_sink.c`

**Bug:** `ij_file_sink_rotation_due` casts `(time_t)(now - sink->last_rotation_epoch)` to `uint64_t` without sign check. If `now < last_rotation_epoch` (NTP step-back), the result wraps to a large value triggering spurious rotation.

- [ ] **Step 1: Read the rotation check code**

Read `src/sinks/ij_file_sink.c` around line 150.

- [ ] **Step 2: Add sign check**

```c
time_t elapsed = now - sink->last_rotation_epoch;
if (elapsed < 0) {
    /* Clock stepped backward — do not rotate */
    return false;
}
```

- [ ] **Step 3: Add test for clock step-back**

In `test_file_sink.c`, set `last_rotation_epoch` to a future time and verify rotation is NOT triggered.

- [ ] **Step 4: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass.

- [ ] **Step 5: Commit**

---

### Task 6: Fix logger config deep copy for syslog_host and logger_name

**Files:**
- Modify: `src/core/ij_logger.c`
- Modify: `src/ij_internal.h` (if needed for owned-string fields)

**Bug:** `ij_logger_init` copies `syslog_host` and `logger_name` pointers by value. If the caller frees the config strings after init, subsequent syslog encoding reads freed memory.

- [ ] **Step 1: Read the current config copy code**

Read `src/core/ij_logger.c` lines 80–110 to understand how config is applied.

- [ ] **Step 2: Deep-copy the string fields**

After `logger->config = *config`, use `strdup` (or a bounded copy) for `logger_name` and `syslog_host`. Free them in `ij_logger_shutdown`. Store the owned copies in separate fields or overwrite the config pointers.

- [ ] **Step 3: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass. No existing test exercises this path directly, but the change is safety-only.

- [ ] **Step 4: Commit**

---

### Task 7: Add ring queue concurrency tests

**Files:**
- Modify: `tests/unit/test_queue.c`

**Gap:** Zero multi-threaded tests despite the ring queue using atomics and mutexes for MPSC patterns.

- [ ] **Step 1: Add MPSC enqueue stress test**

Create a test that spawns 4 threads, each enqueuing 1000 events into a ring of capacity 256. After all threads join, verify `enqueued_events + dropped_events == 4000` and dequeue all remaining events without corruption.

- [ ] **Step 2: Add concurrent enqueue/dequeue test**

Create a test with 2 producer threads and 1 consumer thread. Producers each enqueue 500 events. Consumer dequeues in a loop. After join, verify total dequeued + dropped equals total enqueued.

- [ ] **Step 3: Add empty-dequeue test**

Test `ij_ring_try_dequeue` on an empty ring returns `false` without modifying `out_event`.

- [ ] **Step 4: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass, including under ThreadSanitizer if available.

- [ ] **Step 5: Commit**

---

### Task 8: Add UTF-8 validation unit tests

**Files:**
- Create: `tests/unit/test_utf8.c`
- Modify: `tests/unit/CMakeLists.txt`

**Gap:** Only 1 test for `0xff` byte. No tests for overlong, surrogate, truncated, or boundary cases.

- [ ] **Step 1: Create test file with validation test cases**

Test cases:
- Valid: ASCII, 2-byte (é), 3-byte (日), 4-byte (𝄞), mixed valid
- Invalid: overlong `0xC0 0x80`, surrogate half `0xED 0xA0 0x80`, above U+10FFFF `0xF4 0x90 0x80 0x80`
- Boundary: truncated 2-byte at end, truncated 3-byte at end, truncated 4-byte at end
- Edge: empty string (len=0), exactly 32 bytes (SIMD threshold), 33 bytes with invalid at position 33

- [ ] **Step 2: Register test in CMakeLists.txt**

Add `test_utf8` target and `add_test` call.

- [ ] **Step 3: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass (the OOB fix from Task 1 is required for truncated-sequence tests).

- [ ] **Step 4: Commit**

---

### Task 9: Add SIMD scalar-vs-AVX2 equivalence tests

**Files:**
- Modify: `tests/unit/test_simd_dispatch.c`

**Gap:** Dispatch enum is tested, but SIMD and scalar paths are never compared for identical results.

- [ ] **Step 1: Add JSON escape equivalence test**

Call both `ij_json_escape_find_first_special_scalar` and the dispatched `ij_json_escape_find_first_special` with identical inputs. Verify they return the same position for: all-safe ASCII (64 bytes), special char at every 32-byte boundary position, mixed content.

- [ ] **Step 2: Add UTF-8 validation equivalence test**

Call both `ij_utf8_is_valid_scalar` and `ij_utf8_is_valid_dispatch` with identical inputs covering valid and invalid sequences above 32 bytes.

- [ ] **Step 3: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass on x86 (AVX2 path) and scalar fallback produce same results.

- [ ] **Step 4: Commit**

---

### Task 10: Add encoder buffer overflow tests

**Files:**
- Modify: `tests/unit/test_console_json.c`
- Modify: `tests/unit/test_rfc5424.c`

**Gap:** All encoder tests use large buffers. `IJ_STATUS_ENCODE_ERROR` path is never exercised.

- [ ] **Step 1: Add JSON console encoder overflow test**

Create a valid event, call `ij_json_console_encode` with a buffer of 10 bytes. Verify return is `IJ_STATUS_ENCODE_ERROR`.

- [ ] **Step 2: Add JSON console encoder exact-fit test**

Find the exact size needed for a minimal event and test with a buffer of exactly that size. Verify success.

- [ ] **Step 3: Add RFC 5424 encoder overflow test**

Same pattern for `ij_rfc5424_encode` with an undersized buffer.

- [ ] **Step 4: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass.

- [ ] **Step 5: Commit**

---

### Task 11: Add event validation negative tests

**Files:**
- Modify: `tests/unit/test_event.c`

**Gap:** `ij_validate_event` rejection paths for invalid attribute values are untested.

- [ ] **Step 1: Add NaN/Inf double attribute test**

Create an event with a double attribute set to `NAN`. Call `ij_validate_event`. Verify `IJ_STATUS_INVALID_ARGUMENT`.

- [ ] **Step 2: Add NULL string attribute test**

Create an event with a string attribute where `data = NULL`. Verify rejection.

- [ ] **Step 3: Add oversized string attribute test**

Create an event with `len > IJ_ATTRIBUTE_STRING_MAX_LEN`. Verify rejection.

- [ ] **Step 4: Add unknown attribute kind test**

Create an event with `kind = (ij_attr_value_kind_t)99`. Verify rejection.

- [ ] **Step 5: Add excess attribute count test**

Create an event with `attribute_count > IJ_ATTRIBUTE_COUNT_MAX`. Verify rejection.

- [ ] **Step 6: Build and run tests**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: all tests pass.

- [ ] **Step 7: Commit**

---

### Sprint 13 Exit Criteria

- All 6 bugs fixed
- `ctest --preset clang-debug` passes with all new tests
- `quality.sh` PASS: 14, FAIL: 0 inside Podman
- No public API or ABI changes
