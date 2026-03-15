# RC Memory Ordering Rules For v0.1.0-rc.1

[![C23](https://img.shields.io/badge/C-23-blue)](https://en.cppreference.com/w/c/23)
[![TSAN](https://img.shields.io/badge/LLVM-ThreadSanitizer-blue)](https://clang.llvm.org/docs/ThreadSanitizer.html)
[![SemVer](https://img.shields.io/badge/SemVer-2.0.0-green)](https://semver.org/spec/v2.0.0.html)

## Scope

- Freeze the minimum atomic-ordering contract for the RC queue.
- Make ownership transfer between producers and the consumer explicit.
- Prevent implementation shortcuts that would invalidate concurrency tests later.

## Ownership Transitions

One ring slot moves through these states:

1. free
2. reserved by one producer
3. fully populated by that producer
4. published to the consumer
5. consumed by the single consumer
6. released back to free

No state transition may be observed out of order across threads.

## Required Atomic Rules

- Producer publication of a populated slot must use release semantics.
- Consumer observation of a published slot must use acquire semantics before reading payload data.
- Consumer release of a consumed slot back to reusable state must use release semantics.
- Producers checking slot reusability must use acquire semantics before overwriting a previously consumed slot.
- Ownership transfer across threads must not rely on plain non-atomic flags.

## Allowed Internal Flexibility

- Reservation counters may use monotonic indices or per-slot sequence numbers.
- Ticket reservation may use relaxed or stronger ordering only if publication and reuse boundaries still satisfy the acquire/release rules above.
- The RC contract does not freeze one exact algorithm, but it does freeze the visibility boundaries.

## Ordering Hazards To Prevent

- reading payload bytes before the publishing producer completed the write
- reusing a slot before the consumer released it
- tearing queue metadata across producers
- treating timestamp order as a substitute for publication order
- ABA-style slot reuse confusion caused by missing sequence or generation tracking

## Non-Goals

- Lock-free proof text is not required in the RC docs.
- Wait-free guarantees are not part of the RC.
- Cross-process memory ordering is not part of the RC.
