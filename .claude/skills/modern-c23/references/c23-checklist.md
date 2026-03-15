# Modern C23 Checklist For iojournal

## Prefer

- `nullptr` instead of `NULL`
- `constexpr` for compile-time limits and masks
- `_Static_assert` for ABI, enum, and queue invariants
- `typeof` where it reduces fragile duplication
- `<stdbit.h>` for capacity and power-of-two helpers
- `<stdckdint.h>` for derived size arithmetic
- `__VA_OPT__` for logging macros

## Use Carefully

- `_BitInt` only when exact bit width matters and review clarity stays acceptable
- `auto` only when the inferred type is obvious at the call site
- Unicode helpers only when output contracts need them

## Reject

- Clever inference that hides ownership or width
- Locale-dependent parsing in core paths
- Features that complicate portability without measurable gain
- Macro layers that hide side effects or evaluation order
