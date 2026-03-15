# RC Concurrency Matrix For v0.1.0-rc.1

[![TSAN](https://img.shields.io/badge/LLVM-ThreadSanitizer-blue)](https://clang.llvm.org/docs/ThreadSanitizer.html)
[![ASAN](https://img.shields.io/badge/LLVM-AddressSanitizer-blue)](https://clang.llvm.org/docs/AddressSanitizer.html)
[![C23](https://img.shields.io/badge/C-23-blue)](https://en.cppreference.com/w/c/23)

## Scope

- Define the mandatory concurrency validation matrix for the RC queue model.
- Map queue, overflow, ordering, and ownership risks to future tests.

## Matrix

| Area | Scenario | Required outcome |
| --- | --- | --- |
| empty queue | consumer observes no published slot | no stale payload read |
| single producer | enqueue then dequeue | event preserved exactly once |
| multiple producers | concurrent enqueue into near-empty ring | no metadata corruption, no duplicate dequeue |
| wraparound | head and tail cross ring boundary | no slot aliasing or order regression |
| full ring | producer hits capacity with `DROP_NEW` | `IJ_STATUS_QUEUE_FULL`, existing queue intact |
| repeated overflow | sustained producer pressure | drop counters grow deterministically |
| release and reuse | consumer frees slot and producer reuses it | no read-after-release, no stale payload visibility |
| per-producer ordering | one producer emits A then B | consumer never sees B before A |
| cross-producer ordering | two producers enqueue concurrently | resulting order matches publication order for that run |
| redaction path | event with sensitive values under queue pressure | redaction remains correct and sink-independent |
| shutdown race | producer and shutdown overlap | no use-after-shutdown success path |
| flush race | flush overlaps active producers | no partial drain visibility, no dropped published events |
| bounds enforcement | oversize event under concurrency | deterministic rejection without queue corruption |
| ABA defense | rapid slot reuse cycles | sequence tracking prevents stale-slot confusion |

## Required Negative Coverage

- invalid UTF-8 under concurrent producers
- duplicate attribute keys under concurrent producers
- queue saturation at minimum and default capacities
- publication order versus timestamp order divergence

## Cross-Document Dependencies

- Queue topology comes from `docs/architecture/QUEUE_MODEL.md`.
- Overflow behavior comes from `docs/architecture/BACKPRESSURE_POLICY.md`.
- Ownership and lifetime checks come from `docs/api/OWNERSHIP_AND_LIFETIME.md`.
