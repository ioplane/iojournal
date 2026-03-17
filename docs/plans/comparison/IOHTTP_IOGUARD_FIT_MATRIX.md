# iohttp And ioguard Fit Matrix

## Evaluation Rule

- This document answers whether a library is realistic for `iohttp` or `ioguard`.
- A library can be:
  - `replaceable`: close enough to serve as a direct alternative inside the consumer
  - `partial`: useful comparison point, but with material adoption caveats
  - `unsuitable`: informative only; not a realistic replacement path

## Consumer Fit Matrix

| Library | `iohttp` Fit | `ioguard` Fit | Blocking Gaps | Acceptable Comparison Value | RC Suitability Verdict |
| --- | --- | --- | --- | --- | --- |
| `iojournal` | `replaceable` | `replaceable` | No missing RC surface; remaining risk is now concentrated in the residual `zlog` gap on enabled shared rows, not protocol scope or operability | Canonical baseline | Sprint 11D evidence refresh complete; proceed to Sprint 12 publication hardening |
| `zlog` | `partial` | `partial` | Config-file-first model, no native NDJSON contract, no RFC 5424 structured-data contract, POSIX-focused runtime story | Strong comparator for file-heavy server logging and operational reload behavior | Keep as Tier 1 file-centric baseline; shared-scenario winner, but not a full protocol peer |
| `stumpless` | `partial` | `partial` | No built-in file rotation or retention contract; wider target surface than the RC target may require external policy and extra integration work | Strong comparator for RFC 5424, syslog transport, and structured logging surface | Keep as Tier 1 protocol-centric baseline; closest functional peer and cheaper on enabled shared scenarios |
| `tinylog` | `partial` | `unsuitable` | No native syslog or general network sink surface; Linux or UNIX-only focus; no structured attribute contract | Strong comparator for async file logging, process-safe writes, and archive rotation | Keep as Tier 1 file-throughput baseline only; measured rows are informative, not protocol-equivalent |

## Consumer-Specific Notes

### `iohttp`

- Required baseline:
  - bounded logging cost on hot paths
  - file sink with predictable rotation behavior
  - syslog compatibility for daemon deployments
  - analyzer-friendly build and integration path
- Interpretation:
  - `zlog` is relevant because it is a mature file and syslog comparator.
  - `stumpless` is relevant because it best matches protocol-driven delivery.
  - `tinylog` is relevant only for file-path and async-write comparisons.
  - The current `210134` local shared-scenario results put `iojournal` ahead of `stumpless` and `tinylog` on `enabled_console`, `medium_message`, `medium_message_with_metadata`, `contention_mpsc`, and `append_ndjson`; only `zlog` still sets the stronger raw throughput bar in the enabled shared contract.
  - `iojournal` now wins the shared `disabled_level` row against `zlog`, which removes the hottest no-op adoption concern for `iohttp`.
  - Sprint 11D materially reduced the local hot rows that matter most to `iohttp`: `enabled_console` moved from `1059.62` to `893.62 ns/op`, `medium_message` from `819.13` to `780.70 ns/op`, `medium_message_with_metadata` from `1059.55` to `951.00 ns/op`, `contention_mpsc` from `403.21` to `355.73 ns/op`, and `append_ndjson` from `822.84` to `648.53 ns/op`.
  - the remaining adoption risk for `iohttp` is no longer file-path or contention risk against `stumpless`/`tinylog`; it is the residual enabled-path gap to `zlog`.

### `ioguard`

- Required baseline:
  - machine-consumable logs
  - predictable severity mapping
  - network or syslog delivery for operational forwarding
  - analyzable and policy-driven integration
- Interpretation:
  - `stumpless` is the closest functional comparator because of its RFC 5424 and target breadth.
  - `zlog` remains useful for file-oriented deployments but is structurally weaker for structured contracts.
  - `tinylog` is not a realistic direct replacement because it lacks the target protocol surface needed for the guard use case.
  - current shared-scenario results therefore matter more for `stumpless` than for `tinylog`; the latter is useful as cost pressure on the file path, not as a target-shape substitute.
  - Sprint 11D closed and retained the `ioguard`-relevant gap versus `stumpless` and `tinylog` on all enabled shared rows where protocol contract is comparable; `ioguard` still retains comparable pressure on file-path normalization through `append_file`.
  - `iojournal` remains the only Tier 1 entry with direct protocol/data-shaping alignment and is competitive in enabled shared rows versus `stumpless` and `tinylog`; `zlog` still leads the strongest enabled throughput frontier, so `ioguard` adoption risk remains centered there.

## Release Gate Usage

- This matrix is release-blocking together with `TIER1_FUNCTIONAL_MATRIX.md`.
- `partial` libraries still matter if they provide credible competitive pressure on one major slice of the RC surface.
- `unsuitable` means the library may remain in performance tables only where a normalized scenario is still meaningful.
- This fit matrix and `PERFORMANCE_RESULTS.md` now agree on the refreshed Sprint 11D adoption verdict for `iohttp` and `ioguard`.
