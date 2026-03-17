# ioJournal Bottleneck And Optimization Design

## Scope

- Add one mandatory pre-publish performance sprint before the final `v0.1.0-rc.1` publication gate.
- Make the sprint responsible for bottleneck confirmation, optimization implementation, and evidence refresh.
- Treat `io_uring` as a relevance-first optimization track: measure first, implement only if the measured benefit justifies the added runtime path.
- Treat SIMD the same way: keep a scalar baseline, add architecture-specific fast paths only for functions that profiler evidence proves relevant.

## Current Facts

### Runtime And Perf Facts

- The current RC implementation is still synchronous at sink-delivery time.
- The current dominant cost centers are in event copy, redaction, string shaping, and JSON or NDJSON encoding.
- The current release-facing comparison evidence already shows that sink write cost is not the dominant hotspot for the measured shared scenarios.

### Host Kernel Facts

Observed on `2026-03-16`:

- host kernel: `6.12.0-107.59.3.4.el10uek.x86_64`
- `CONFIG_IO_URING=y`
- `/proc/sys/kernel/io_uring_disabled=0`
- host-side `io_uring_setup()` probe succeeds

Conclusion:

- the host is capable of running `io_uring`

### Podman Facts

Observed on `2026-03-16`:

- default Podman containers run with `Seccomp: 2`
- inside the default container, `io_uring_setup()` returns `ENOSYS`
- inside `podman run --security-opt seccomp=unconfined`, the same `io_uring_setup()` probe succeeds

Conclusion:

- the blocker is not the kernel
- the blocker is the default Podman seccomp profile
- any mandatory `io_uring` evaluation or implementation sprint must define an official container launch mode for `io_uring` lanes

## io_uring Decision Model

`io_uring` is not automatically justified just because the kernel supports it.

The sprint must answer two separate questions:

1. Is `io_uring` relevant for the current measured bottlenecks?
2. If it is relevant, which sink path should adopt it first?

Decision rules:

- Do not introduce `io_uring` into every sink path by default.
- Evaluate file sink first.
- Evaluate syslog network paths second and only if the file-sink study does not absorb the full optimization budget.
- Keep synchronous fallback paths in place.
- Do not let `io_uring` become the only supported sink runtime for `v0.1.0-rc.1`.

## SIMD Decision Model

SIMD must remain narrow and measurable.

The sprint should not target every possible ISA extension. It should use one portable baseline and one practical fast path per architecture family:

- `x86_64`: scalar baseline plus `AVX2`
- `arm64`: scalar baseline plus `NEON`

Deferred from this sprint:

- `AVX-512`
- `SVE`
- `SVE2`

Reason:

- these wider ISA families materially narrow deployment coverage
- they increase implementation and verification cost
- they are not needed for the first release-blocking optimization pass

## Candidate SIMD Surfaces

SIMD is relevant only where the current profiles suggest text-heavy scanning or classification work:

- JSON string escaping and quote or control-character scan
- redact-key matching and denylist scan
- UTF-8 validation
- fixed-delimiter or ASCII-classification helpers used in builders or encoders

SIMD is not a primary candidate for:

- queue publication logic
- sink write calls themselves
- system-call dominated paths

## New Mandatory Sprint

Add one new release-blocking sprint between the current comparison sprint and the final publication sprint.

Recommended sequence:

- Sprint 10: functional and performance comparison
- Sprint 11A: bottleneck and performance optimization
- Sprint 11: final RC decision and publication

This avoids renumbering the existing final-publication plan file while making the new sprint explicit and mandatory.

## Sprint Outputs

The new sprint must produce:

- one official Podman launch mode for `io_uring` perf lanes
- hotspot confirmation artifacts tied to the current benchmark scenarios
- one `io_uring` relevance report
- one `SIMD` relevance report
- implementation only for the confirmed profitable targets
- refreshed benchmark, profiler, and comparison evidence after optimization
- a final accept or reject decision for every deferred optimization surface that was evaluated

## Non-Goals

- no mandatory `AVX-512` or `SVE` support in this sprint
- no replacement of the scalar baseline with ISA-specific code
- no uncontrolled expansion of `io_uring` across every sink without evidence
- no removal of the synchronous fallback path during the first optimization sprint
