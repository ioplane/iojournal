---
name: modern-c23
description: Use when writing modern C23 code for iojournal, especially hot-path logging, queueing, checked arithmetic, macro design, and concurrency-sensitive code. Covers approved language features, library helpers, and project-specific constraints.
---

# Modern C23

## Overview

Use this skill to choose C23 features deliberately instead of as novelty. Favor type safety, checked size math, compile-time validation, and explicit ownership.

## Preferred Features

- `nullptr` over `NULL`
- `constexpr` for compile-time constants
- `typeof` when it removes fragile repetition without hiding ownership
- `[[nodiscard]]` on public error-returning APIs
- `_Static_assert` for invariants that must not drift silently
- `<stdbit.h>` for ring and power-of-two helpers
- `<stdckdint.h>` for size arithmetic on untrusted or derived input
- `__VA_OPT__` for clean logging macro fronts

## Use With Care

- `_BitInt` only when exact bit width pays for its complexity and ABI remains controlled.
- `auto` and aggressive inference only when the resulting code stays obvious at review time.
- UTF-8 and Unicode helpers only when they fit the log/output contract.

## Avoid

- Hidden allocations in convenience wrappers.
- Locale-sensitive parsing or formatting in core paths.
- C23 features that complicate ABI or portability without a measurable benefit.
- Clever macros that hide side effects in logging call sites.

## Workflow

1. Pick the narrowest feature that improves correctness or clarity.
2. Guard size and capacity math with checked helpers.
3. Keep public contracts explicit, especially around ownership and failure.
4. Re-check that the hot path stayed bounded.

## References

- `references/c23-checklist.md`
