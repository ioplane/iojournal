---
name: iojournal-coding-standards
description: Use when writing or reviewing any C code in iojournal, especially public headers, hot-path logging, queueing code, sink implementations, filters, and concurrency-sensitive tests. Mandatory for all new and modified C files.
---

# Iojournal Coding Standards

## Overview

Use this skill to apply the repository's coding rules without letting performance shortcuts break safety, API clarity, or hot-path guarantees.

## File Placement

- Keep public declarations in `include/iojournal/`.
- Keep internal-only declarations in `src/`.
- Keep tests under `tests/unit/` with `test_<area>.c` names.

## Naming

- Functions: `ij_*`
- Types: `*_t`
- Macros and enum values: `IJ_*`
- Prefer narrow, purpose-specific modules over mixed-responsibility translation units.

## Style Rules

- Follow `.clang-format`: 4 spaces, Linux braces, 100 columns, right-aligned pointer stars.
- Prefer comments that explain why, not what.
- Keep public error-returning APIs `[[nodiscard]]`.
- Use checked arithmetic when input-derived sizes can overflow.

## Safety Rules

- No hidden allocation in `ij_log` or other hot-path producers.
- Do not bypass redaction or secret-handling defaults.
- Avoid blocking operations in producer paths.
- Keep concurrency code explicit about ownership, ordering, and queue state transitions.

## Test Expectations

- Add or update tests for every behavior change.
- Prefer sanitizer coverage for queueing, ownership, and memory-safety work.
- For sink and encoder changes, verify both formatting rules and failure handling.

## References

- `references/coding-checklist.md`
