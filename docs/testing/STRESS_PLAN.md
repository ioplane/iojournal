# RC Stress Plan For v0.1.0-rc.1

[![TSAN](https://img.shields.io/badge/LLVM-ThreadSanitizer-blue)](https://clang.llvm.org/docs/ThreadSanitizer.html)
[![ASAN](https://img.shields.io/badge/LLVM-AddressSanitizer-blue)](https://clang.llvm.org/docs/AddressSanitizer.html)
[![C23](https://img.shields.io/badge/C-23-blue)](https://en.cppreference.com/w/c/23)

## Scope

- Define the execution strategy for Sprint 05 and later concurrency verification.
- Freeze the minimum sanitizer and stress evidence expectations for the RC.

## Required Test Modes

| Mode | Requirement |
| --- | --- |
| debug unit tests | baseline queue correctness |
| ASAN | bounds and use-after-free checks |
| TSAN | race detection on producer, consumer, flush, and shutdown paths |
| long-loop stress | repeated enqueue/dequeue and overflow pressure |

## Required Stress Dimensions

- producer counts: 1, 2, 4, 8
- capacities: 256 and 1024
- sink mode: stub consumer, file-oriented mock, syslog-oriented mock
- redaction mode: disabled and enabled
- payload size: minimal event, mid-size event, maximum RC event

## Schedule Strategy

- Use deterministic seedable randomized schedules where supported by the test harness.
- Insert controlled yields and consumer delays to widen producer and consumer interleavings.
- Run long-loop stress for at least 100000 successful enqueue attempts per configuration.
- Run overflow-focused stress with sustained pressure until at least 1000 `IJ_STATUS_QUEUE_FULL` outcomes are observed.

## Evidence Format

- Record configuration tuple: producer count, ring capacity, redaction mode, payload profile, sanitizer mode.
- Record event totals: accepted, dropped, consumed, and flushed.
- Record whether any sanitizer finding occurred.
- Preserve golden summaries for the smallest and default-capacity configurations.

## Exit Conditions

- No sanitizer findings in the mandatory matrix.
- No queue corruption, duplicate dequeue, or stale-slot read in the mandatory matrix.
- Overflow outcomes remain deterministic under the `DROP_NEW` policy.
- Redaction behavior remains stable under stressed producer schedules.

## Non-Goals

- Throughput benchmarking is not part of this document.
- Hardware-specific profiling methodology is not part of this document.
- Post-RC exporter stress coverage is not part of this document.
