# iojournal Coding Checklist

## Public API

- Keep public declarations under `include/iojournal/`.
- Use `ij_*` for functions and `IJ_*` for macros and enum values.
- Mark public fallible APIs `[[nodiscard]]`.

## Hot Path

- No heap allocation in `ij_log` or equivalent producer paths.
- No blocking I/O, lock contention, or hidden formatting explosions.
- Use checked arithmetic for size and capacity math.

## Style

- Follow `.clang-format`.
- Keep functions small and ownership explicit.
- Prefer comments that explain invariants, contracts, and why a branch exists.

## Tests

- Add or update unit tests for each behavior change.
- Use sanitizers when queueing, ownership, or memory-safety behavior changes.
- Cover failure modes for sink and encoder work, not only success paths.
