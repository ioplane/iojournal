# Superseded Sprint File

This file is superseded by `2026-03-16-sprint-12-final-rc-decision-and-publication.md`.
Keep it only as a historical planning artifact from the earlier sequence before Sprint 11B was inserted.

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** make the final `v0.1.0-rc.1` publish decision after feature, benchmark, and competitor-comparison evidence are complete.

**Architecture:** this sprint must not add new runtime features. It may only finalize release evidence, publish criteria, and release mechanics.

**Tech Stack:** release scripts, benchmark artifacts, comparison docs, changelog, release notes, GitHub release flow.

---

### Task 1: Freeze the publish criteria

**Files:**
- Modify: `docs/en/03-release-candidate-checklist.md`
- Modify: `docs/ru/03-release-candidate-checklist.md`
- Create: `docs/en/04-rc1-publish-decision.md`
- Create: `docs/ru/04-rc1-publish-decision.md`

- [ ] **Step 1: Add comparison gate criteria**

Expected: publish criteria reference benchmark and comparison evidence explicitly.

- [ ] **Step 2: Record pass/fail evidence**

Expected: decision docs point to quality, release, and comparison artifacts.

- [ ] **Step 3: Verify docs surface**

Run: `python3 scripts/lint-docs.py`
Expected: stable docs pass lint.

### Task 2: Final release execution

**Files:**
- Modify: `scripts/run-release-candidate.sh`
- Modify: `scripts/build-release-assets.sh`
- Modify: `scripts/render-release-notes.sh`
- Modify: `CHANGELOG.md`

- [ ] **Step 1: Run the final release candidate path**

Run: `bash scripts/run-release-candidate.sh`
Expected: RC evidence is regenerated from the final state.

- [ ] **Step 2: Build release assets**

Run: `bash scripts/build-release-assets.sh v0.1.0-rc.1`
Expected: release assets exist for the publish decision.

- [ ] **Step 3: Finalize release notes inputs**

Expected: changelog and notes align with the final RC evidence set.

### Task 3: Make the publish decision explicit

**Files:**
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/README.md`

- [ ] **Step 1: Record final readiness**

Expected: the roadmap and plan index point to the final decision artifacts.

- [ ] **Step 2: Verify the full publish gate**

Run: `bash scripts/quality.sh && bash scripts/run-release-candidate.sh`
Expected: the publish decision can be made without unstated assumptions.
