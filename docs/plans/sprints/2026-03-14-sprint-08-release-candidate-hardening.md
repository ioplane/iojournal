# Sprint 08: RC Infrastructure And Evidence Hardening Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** harden the implemented RC feature surface into a measurable release-candidate infrastructure package with verification evidence, examples, packaging, and explicit publish-gate inputs.

**Architecture:** this sprint should not add new features. It should only harden, document, verify, and package the existing RC surface. The final publish/no-publish decision moves to Sprint 11 after benchmark and comparison evidence exist.

**Tech Stack:** release scripts, examples, changelog, release notes, docs, verification artifacts.

---

### Task 1: Harden release scripts and evidence collection

**Files:**
- Modify: `scripts/quality.sh`
- Modify: `scripts/run-release-gate.sh`
- Modify: `scripts/run-coverage.sh`
- Modify: `scripts/run-release-candidate.sh`
- Modify: `scripts/build-release-assets.sh`
- Modify: `scripts/render-release-notes.sh`

- [ ] **Step 1: Write the failing release-gate check**

Run: `bash scripts/run-release-gate.sh`
Expected: exposes missing RC hardening steps before the sprint changes.

- [ ] **Step 2: Wire the full RC verification surface**

Expected: quality, tests, docs, examples, coverage, and artifact generation are all invoked.

- [ ] **Step 3: Verify release candidate run**

Run: `bash scripts/run-release-candidate.sh`
Expected: release-candidate evidence is generated successfully.

### Task 2: Prepare RC documentation and examples

**Files:**
- Modify: `README.md`
- Create: `docs/en/03-release-candidate-checklist.md`
- Create: `docs/ru/03-release-candidate-checklist.md`
- Modify: `CHANGELOG.md`
- Modify: `examples/*`

- [ ] **Step 1: Update top-level docs**

Expected: README and release checklist describe the RC feature set and its limits.

- [ ] **Step 2: Finalize examples**

Expected: console, file, UDP syslog, and TCP syslog examples are runnable and documented.

- [ ] **Step 3: Update changelog and release notes inputs**

Expected: externally visible RC behavior is recorded.

- [ ] **Step 4: Verify docs surface**

Run: `python3 scripts/lint-docs.py`
Expected: stable docs surface passes lint.

### Task 3: Make the publish decision explicit

**Files:**
- Create: `docs/en/04-rc1-publish-decision.md`
- Create: `docs/ru/04-rc1-publish-decision.md`
- Modify: `docs/plans/ROADMAP.md`

- [ ] **Step 1: Define publish criteria**

Expected: all RC gates are listed and measurable.

- [ ] **Step 2: Record pass/fail evidence**

Expected: decision doc references artifacts from release-gate and release-candidate runs.

- [ ] **Step 3: Verify the final RC gate**

Run: `bash scripts/quality.sh && bash scripts/run-release-candidate.sh && bash scripts/build-release-assets.sh v0.1.0-rc.1`
Expected: RC artifacts are present and the publish decision can be made from evidence.
