# iojournal Next Optimization Tranche Risk Assessment

## Objective

Define the next contract-preserving optimization tranche after Sprint 11C and rank the candidate
changes by professional delivery risk, security impact, and expected performance leverage.

## Non-Negotiable Controls

- Public API, ownership rules, and synchronous RC semantics must not change.
- Event acceptance and rejection behavior must remain byte-for-byte compatible with the current
  contract.
- Redaction must not introduce false negatives.
- JSON, NDJSON, and RFC 5424 emitted text must remain exact.
- Every accepted optimization must survive the full containerized quality gate and refreshed quiet
  benchmark evidence.

## Risk Model

Scale:

- Likelihood: `Low`, `Medium`, `High`
- Impact: `Moderate`, `Major`, `Critical`
- Inherent risk: judgment before controls
- Residual risk: judgment after the required controls and tests

## Candidate Matrix

| Candidate | Expected Gain | Likelihood | Impact | Inherent Risk | Main Failure Modes | Required Controls | Residual Risk | Decision |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Fixed-fragment JSON fast path for constant keys and level text | Medium | Low | Major | Medium | changed emitted JSON text, punctuation drift, wrong level text, hidden field-order drift | exact-output tests, source-of-record benchmark rerun, no dynamic-field behavior changes | Low | Execute first |
| Additional event-copy tightening inside current ownership model | Medium | Medium | Critical | High | acceptance/rejection drift, duplicate-key drift, UTF-8 drift, ownership bugs | failure-first tests, exact parity on invalid inputs, stress on redaction and dispose path | Medium | Execute after JSON fragment slice |
| Redaction dispatch tightening beyond current buckets | Low to Medium | Medium | Critical | High | false-negative redaction, suffix-matching drift, case-folding drift | exhaustive key corpus tests, dotted-suffix parity tests, explicit security sign-off in docs | Medium to High | Keep in tranche, but not first |
| Common event-shape specialized fast path | High | High | Major | High | two-path drift, optional-field divergence, hidden maintenance burden, partial behavior skew under rare payload shapes | exact-output differential tests across generic and specialized paths, explicit shape predicate contract, fallback path invariants | High | Defer unless prior slices are insufficient |
| Timestamp micro-optimizations beyond current fixed-format path | Low | Low | Moderate | Low | RFC3339 edge-case drift, year-range drift, timezone regression | exact timestamp fixtures and boundary tests | Low | Optional final slice |

## Professional Risk Position

### 1. Fixed-fragment JSON fast path

This is the best risk-adjusted move.

- It attacks a measured hotspot: repeated builder churn in `ij_json_console_encode`.
- It does not require weaker validation or weaker redaction.
- The risk surface is narrow and highly testable because the keys and punctuation are constant.

### 2. Event-copy tightening

This remains attractive, but it is materially riskier than the JSON fragment slice.

- The likely win is real because `ij_event_copy_from_input` is still a first-order hotspot.
- The hidden downside is semantic drift on invalid inputs and lifetime rules.
- This area touches correctness more directly than JSON fragment specialization.

### 3. Redaction tightening

This has poor risk tolerance because the downside is security-sensitive.

- Even a small false-negative rate is unacceptable.
- The residual risk remains meaningful even with good tests because the policy surface is
  security-critical.

### 4. Common event-shape specialized path

This is the highest structural risk in the set.

- It can be fast.
- It also creates dual behavior paths and long-term maintenance risk.
- It should be treated as a last resort before publication, not as the next default step.

### 5. Timestamp refinements

This is safe, but it is not the highest-value move.

- It should be used only if a low-risk cleanup falls out naturally from other work.

## Recommended Execution Order

1. Fixed-fragment JSON fast path
2. Event-copy tightening with exact semantic locks
3. Redaction tightening only if evidence still justifies it
4. Timestamp cleanup if still worthwhile
5. Common event-shape specialization only if the remaining `zlog` gap is still release-relevant

## Release Gates For This Tranche

An optimization slice is accepted only if all of the following are true:

- full Podman quality gate passes
- stable docs lint passes
- quiet benchmark run improves or at least does not regress the targeted rows
- profiler evidence confirms the targeted hotspot actually shrank
- no consumer-fit downgrade appears in `IOHTTP_IOGUARD_FIT_MATRIX.md`

## Explicit No-Go Conditions

Abort or roll back a slice immediately if any of these appear:

- changed emitted JSON or NDJSON text
- changed accepted or rejected event surface
- changed redaction decisions
- improvement on one row but material regression on `medium_message_with_metadata` or
  `append_ndjson`
- optimizer-specific tricks that widen the ABI or require non-portable runtime assumptions

## Chosen Tranche

Create a new release-blocking Sprint 11D:

- focus: contract-preserving fast paths only
- first implementation slice: fixed-fragment JSON fast path and prequoted level strings
- deferred inside the sprint unless evidence still demands them: common event-shape specialization
