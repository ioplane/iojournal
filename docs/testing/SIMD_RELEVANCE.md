# SIMD Relevance Decision For Sprint 11A

[![AVX2](https://img.shields.io/badge/x86__64-AVX2-4c8bf5)](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)
[![Arm NEON](https://img.shields.io/badge/arm64-NEON-4c8bf5)](https://developer.arm.com/architectures/instruction-sets/intrinsics/)
[![Benchmark Methodology](https://img.shields.io/badge/Benchmark-Methodology-4c8bf5)](/opt/projects/repositories/iojournal/docs/testing/BENCHMARK_METHODOLOGY.md)
[![Mermaid](https://img.shields.io/badge/Mermaid-requirementDiagram-blue)](https://mermaid.js.org/syntax/requirementDiagram.html)

## Scope

- Record the Sprint 11A keep or drop decision for the SIMD hotspot targets.
- Bind the decision to one traceable microbenchmark artifact set.
- Keep only the measured profitable SIMD targets in the RC-active path.

```mermaid
requirementDiagram
    requirement simd_measure {
        id: SIMD-1
        text: Keep or drop decisions must cite one measured scalar row and one measured ISA row
        risk: high
        verifymethod: analysis
    }

    requirement simd_scalar {
        id: SIMD-2
        text: Scalar logic remains the canonical correctness baseline
        risk: high
        verifymethod: test
    }

    requirement simd_rc {
        id: SIMD-3
        text: Unprofitable or unmeasured SIMD paths do not stay in the RC-active dispatch path
        risk: high
        verifymethod: analysis
    }
```

## Source Of Record

- benchmark artifact: [`20260316-145708/bench_simd_scan.tsv`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-145708/bench_simd_scan.tsv)
- manifest: [`20260316-145708/manifest.md`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-145708/manifest.md)

Environment:
- host architecture: `x86_64`
- active measured ISA: `AVX2`
- `NEON` code paths are compiled as arm64-specific implementations but are not measured in the current environment

## Measured Rows

| Scenario | Iterations | ns/op | Interpretation |
| --- | --- | --- | --- |
| `json_escape_scalar` | `400000` | `37.75` | scalar baseline for JSON escape scanning |
| `json_escape_active` | `400000` | `6.49` | current RC-active path on this host |
| `json_escape_x86_avx2` | `400000` | `4.53` | explicit `AVX2` path |
| `redact_match_scalar` | `400000` | `4.15` | scalar baseline for key matching |
| `redact_match_active` | `400000` | `4.72` | current RC-active path stays scalar |
| `redact_match_x86_avx2` | `400000` | `7.79` | explicit `AVX2` path |
| `utf8_scalar` | `400000` | `21.46` | scalar baseline for UTF-8 validation |
| `utf8_active` | `400000` | `14.57` | current RC-active path on this host |
| `utf8_x86_avx2` | `400000` | `14.87` | explicit `AVX2` path |

## x86_64 AVX2 Verdicts

| Target | Decision | Reason |
| --- | --- | --- |
| JSON escape scan | keep | explicit `AVX2` row improves from `37.75` to `4.53 ns/op`, about `8.33x` faster than scalar |
| UTF-8 validation fast path | keep | active `AVX2` row improves from `21.46` to `14.57 ns/op`, about `1.47x` faster than scalar |
| redaction key matching | drop | explicit `AVX2` row regresses from `4.15` to `7.79 ns/op`, about `1.88x` slower than scalar |

Implementation consequence:
- `x86_64 AVX2` remains active only for JSON escape scanning and UTF-8 validation.
- Redaction matching remains scalar in the RC-active path.

## arm64 NEON Verdicts

| Target | Decision | Reason |
| --- | --- | --- |
| JSON escape scan | drop from RC-active path for now | implementation exists, but there is no arm64 benchmark evidence in the current release artifact set |
| UTF-8 validation fast path | drop from RC-active path for now | implementation exists, but there is no arm64 benchmark evidence in the current release artifact set |
| redaction key matching | drop from RC-active path for now | unmeasured and already a losing target on x86 `AVX2` |

Implementation consequence:
- `NEON` source files remain as bounded implementation experiments.
- `NEON` is not part of the current RC-active dispatch policy until arm64 evidence exists.

## RC-Active Dispatch Policy

| Hotspot | RC-active kind on current measured host | Notes |
| --- | --- | --- |
| JSON escape scan | `x86_avx2` | active only on measured `x86_64 AVX2` hosts |
| redaction key matching | `scalar` | SIMD variant rejected for RC |
| UTF-8 validation | `x86_avx2` | active only on measured `x86_64 AVX2` hosts |

## Conclusion

- SIMD is relevant for the current bottleneck inventory, but only selectively.
- JSON escape scanning is the strongest SIMD candidate in the current code base.
- UTF-8 validation has a smaller but still positive gain on the measured `x86_64 AVX2` host.
- Redaction matching is not a profitable SIMD target for the current RC path.
- arm64 `NEON` remains out of the RC-active path until direct arm64 measurements exist.
