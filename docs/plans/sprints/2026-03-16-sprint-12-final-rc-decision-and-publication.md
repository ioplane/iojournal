# Sprint 12: Final RC Decision And Publication Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** make the final `v0.1.0-rc.1` publish decision after feature, comparison, and optimization evidence are complete, including Sprint 11D.

**Architecture:** this sprint must not add new runtime features. It may only finalize release evidence, publish criteria, and release mechanics.

**Tech Stack:** release scripts, benchmark artifacts, comparison docs, changelog, release notes, GitHub release flow.

---

### Task 0: Handoff continuity pack for next AI

**Files:**
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
- Modify: `docs/plans/sprints/README.md`
- Modify: `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- Modify: `docs/plans/comparison/RAW_ARTIFACTS.md`
- Modify: `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`
- Modify: `docs/plans/comparison/TIER1_FUNCTIONAL_MATRIX.md`
- Modify: `docs/en/06-rc1-publish-decision.md`
- Modify: `docs/ru/06-rc1-publish-decision.md`
- Modify: `docs/en/05-release-candidate-checklist.md`
- Modify: `docs/ru/05-release-candidate-checklist.md`
- Modify: `CHANGELOG.md`

- [x] **Step 1: Write handoff state block**

Expected:
- current source-of-record IDs (benchmarks and profiling) are written to the sprint task notes
- explicit publish status and blocker list are written in both language decision docs
- next commands for the next AI are explicit and prioritized

Done: updated evidence IDs to 11D source-of-record across `06-rc1-publish-decision.md` (en/ru), `05-release-candidate-checklist.md` (en/ru), and `CLAUDE.md`. Structural optimization gate now `PASS`. RC run and release assets marked `FAIL` pending regeneration.

- [x] **Step 2: Run mandatory verification in Podman**

Run:

`podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`

`python3 scripts/lint-docs.py`

Expected: pass for all touched docs and scripts.

Done: `lint-docs.py` PASS, `quality.sh` PASS: 14, FAIL: 0, SKIP: 0.

- [x] **Step 3: Run release gate scripts as a unit**

Run:

`bash scripts/run-release-candidate.sh`

`bash scripts/build-release-assets.sh v0.1.0-rc.1`

`bash scripts/render-release-notes.sh`

Expected:
- release assets and notes are regenerated from current state
- evidence references in decision/docs plans remain internally consistent
- output is appended to handoff notes for the next AI

Done: RC run `20260316T212052Z-d52d88e` PASS: 14, FAIL: 0. Release assets regenerated: `dist/iojournal-v0.1.0-rc.1.tar.gz` (186K), `dist/iojournal-v0.1.0-rc.1-verification.tar.gz` (12K), `dist/iojournal-v0.1.0-rc.1.sha256`, `dist/RELEASE_NOTES.md`. Both en/ru publish decision docs updated with new RC run ID and PASS status for RC run and release assets gates.

### Task 1: Freeze the publish criteria

**Files:**
- Create: `docs/en/05-release-candidate-checklist.md`
- Create: `docs/ru/05-release-candidate-checklist.md`
- Create: `docs/en/06-rc1-publish-decision.md`
- Create: `docs/ru/06-rc1-publish-decision.md`

- [x] **Step 1: Add comparison and optimization gate criteria**

Expected: publish criteria reference benchmark, comparison, and optimization evidence explicitly.

- [x] **Step 2: Record pass/fail evidence**

Expected: decision docs point to quality, release, comparison, and optimization artifacts.

- [x] **Step 3: Verify docs surface**

Run: `python3 scripts/lint-docs.py`
Expected: stable docs pass lint.

### Task 2: Final release execution

**Files:**
- Modify: `scripts/run-release-candidate.sh`
- Modify: `scripts/build-release-assets.sh`
- Modify: `scripts/render-release-notes.sh`
- Modify: `CHANGELOG.md`

- [x] **Step 1: Run the final release candidate path**

Run: `bash scripts/run-release-candidate.sh`
Expected: RC evidence is regenerated from the final state.

- [x] **Step 2: Build release assets**

Run: `bash scripts/build-release-assets.sh v0.1.0-rc.1`
Expected: release assets exist for the publish decision.

- [x] **Step 3: Finalize release notes inputs**

Expected: changelog and notes align with the final RC evidence set.

### Task 3: Make the publish decision explicit

**Files:**
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/README.md`

- [x] **Step 1: Record final readiness**

Expected: the roadmap and plan index point to the final decision artifacts.

- [x] **Step 2: Verify the full publish gate**

**Current status:** Sprint 11D is complete and all local gates are green. The explicit decision is `NO-PUBLISH` pending the final remote workflow gate on a pushed revision. Active RC run: `20260316T212052Z-d52d88e`.

Run: `bash scripts/quality.sh && bash scripts/run-release-candidate.sh`
Expected: the publish decision can be made without unstated assumptions.
