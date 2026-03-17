[![Semantic Versioning](https://img.shields.io/badge/SemVer-2.0.0-3da639)](https://semver.org/)
[![Keep a Changelog](https://img.shields.io/badge/Keep%20a%20Changelog-1.1.0-8a2be2)](https://keepachangelog.com/en/1.1.0/)
[![GitHub Actions](https://img.shields.io/badge/GitHub%20Actions-release%20gate-181717)](https://docs.github.com/en/actions)

# Release Candidate Checklist

## Scope

- This document defines the minimum publication gate for `v0.1.0-rc.1`.
- All release decisions must cite concrete local or published artifacts.
- A missing owner, missing remote validation, or missing artifact is a release blocker.

```mermaid
requirementDiagram
    requirement local_quality {
        id: RC-LOCAL
        text: Local quality gate must pass
        risk: medium
        verifymethod: test
    }
    requirement comparison_gate {
        id: RC-COMPARE
        text: Functional and performance comparison evidence must be current
        risk: medium
        verifymethod: analysis
    }
    requirement optimization_gate {
        id: RC-OPT
        text: Optimization evidence must be complete
        risk: medium
        verifymethod: analysis
    }
    requirement governance_gate {
        id: RC-GOV
        text: CODEOWNERS and remote workflow state must be valid
        risk: high
        verifymethod: inspection
    }
```

## Target

| Field | Value |
| --- | --- |
| Release line | `v0.1.0-rc.1` |
| Release type | first public release candidate |
| Current local benchmark source | [`20260316-210134`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134) |
| Current local profiler source | [`20260316-210204`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204) |
| Current Tier 1 comparison source | [`20260316-150320`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320) |

## Publication Gates

| Gate | Requirement | Evidence | Status Rule |
| --- | --- | --- | --- |
| Local quality | `scripts/quality.sh` passes in the Podman development image | `dist/release-candidate/runs/<RUN_ID>/release-gate.txt` plus local command output | `PASS` only if the run is green on the final workspace state |
| Stable docs | numbered English and Russian docs exist for the publication gate and decision | this document plus `06-rc1-publish-decision.md` in both languages | `PASS` only if `python3 scripts/lint-docs.py` is green |
| Functional comparison | Sprint 10 comparison pack remains complete | [`PERFORMANCE_RESULTS.md`](/opt/projects/repositories/iojournal/docs/plans/comparison/PERFORMANCE_RESULTS.md), [`TIER1_FUNCTIONAL_MATRIX.md`](/opt/projects/repositories/iojournal/docs/plans/comparison/TIER1_FUNCTIONAL_MATRIX.md), [`IOHTTP_IOGUARD_FIT_MATRIX.md`](/opt/projects/repositories/iojournal/docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md) | `PASS` only if Tier 1 evidence is current and internally consistent |
| Optimization evidence | Sprint 11A and Sprint 11B evidence is complete and consistent | [`IO_URING_RELEVANCE.md`](/opt/projects/repositories/iojournal/docs/testing/IO_URING_RELEVANCE.md), [`SIMD_RELEVANCE.md`](/opt/projects/repositories/iojournal/docs/testing/SIMD_RELEVANCE.md), [`PERFORMANCE_RESULTS.md`](/opt/projects/repositories/iojournal/docs/plans/comparison/PERFORMANCE_RESULTS.md) | `PASS` only if the active benchmark and profiler source-of-record artifacts are cited explicitly |
| Release candidate run | `scripts/run-release-candidate.sh` regenerates the final local evidence pack | `dist/release-candidate/runs/<RUN_ID>/summary.md` and `latest.txt` | `PASS` only if the run is produced from the final workspace state |
| Release assets | source archive, verification archive, checksums, and release notes are generated; zip and docs archive remain optional | `dist/` contents for the target tag | `PASS` only if all mandatory artifacts exist |
| Repository ownership | `.github/CODEOWNERS` names a real GitHub user or team | `.github/CODEOWNERS` | `PASS` only if no placeholder text remains |
| Remote gate | GitHub workflow state exists for the final pushed revision | GitHub Actions runs on the final pushed commit | `PASS` only if the publish target revision is pushed and its required workflows are green |

## Exit Rule

- Publish only if every gate above is `PASS`.
- Record the decision in `06-rc1-publish-decision.md`.
- If any gate is not `PASS`, the decision must be `NO-PUBLISH`.
