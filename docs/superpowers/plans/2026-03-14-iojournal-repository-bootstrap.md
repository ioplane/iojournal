# iojournal Repository Bootstrap Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** bootstrap `iojournal` with shared repository infrastructure from `iohttpparser` and replace the ad hoc local notes with proper repository-local skills built from `docs/tmp`.

**Architecture:** reuse the safe repository baseline from `iohttpparser`, but treat every transferred file as a template that must be adapted for a logging library. Keep the new local skills compact in `SKILL.md` and move detail into `references/`.

**Tech Stack:** Markdown, GitHub Actions YAML, Bash, Python helper scripts, repository-local skill metadata.

---

## Chunk 1: Repository Baseline

### Task 1: Snapshot the transferable baseline and target gaps

**Files:**
- Modify: `docs/superpowers/specs/2026-03-14-iojournal-repository-bootstrap-design.md`
- Test: local path checks only

- [ ] **Step 1: Record the current target state**

Run: `find . -maxdepth 2 \\( -path './.github' -o -path './.claude/skills' -o -name 'AGENTS.md' -o -name 'CLAUDE.md' -o -name 'CODEX.md' -o -path './scripts' \\) | sort`
Expected: shows the current minimal `iojournal` baseline before transfer.

- [ ] **Step 2: Record the source baseline**

Run: `cd /opt/projects/repositories/iohttpparser && find .github .claude/skills scripts -maxdepth 3 -type f | sort`
Expected: shows the source files available for selective transfer.

- [ ] **Step 3: Confirm parser-specific strings that must not survive**

Run: `cd /opt/projects/repositories/iohttpparser && rg -n 'iohttpparser|ihtp|parser' .github AGENTS.md CLAUDE.md CODEX.md`
Expected: identifies all strings that require adaptation or removal.

### Task 2: Transfer and adapt `.github/`, root instructions, and required scripts

**Files:**
- Create or modify: `.github/**`
- Create or modify: `AGENTS.md`
- Create or modify: `CLAUDE.md`
- Create or modify: `CODEX.md`
- Create or modify: `scripts/quality.sh`
- Create or modify: `scripts/lint-docs.py`
- Create or modify: `scripts/run-release-gate.sh`
- Create or modify: `scripts/run-coverage.sh`
- Create or modify: `scripts/run-release-candidate.sh`
- Create or modify: `scripts/build-release-assets.sh`
- Create or modify: `scripts/render-release-notes.sh`

- [ ] **Step 1: Copy the source files into `iojournal`**

Run: local copy commands from `iohttpparser` into `iojournal`
Expected: the file set exists locally before content adaptation.

- [ ] **Step 2: Rewrite all project identifiers and repository rules**

Expected changes:
- `iohttpparser` -> `iojournal`
- `ihtp_` / `IHTP_` -> `ij_` / `IJ_`
- parser-layer rules -> logging-library rules
- `IOHTTPPARSER_*` options -> `IOJOURNAL_*`

- [ ] **Step 3: Remove workflows or steps that depend on non-transferred parser assets**

Expected: every workflow references only scripts and options that exist in `iojournal`.

- [ ] **Step 4: Verify transferred paths are real**

Run: `rg -n 'run-release-gate|run-coverage|run-release-candidate|build-release-assets|quality\\.sh|lint-docs\\.py' .github scripts`
Expected: every referenced path resolves inside the repository.

## Chunk 2: Local Skills

### Task 3: Create the new skill skeletons with the skill-creator tooling

**Files:**
- Create: `.claude/skills/iojournal-architecture/**`
- Create: `.claude/skills/iojournal-coding-standards/**`
- Create: `.claude/skills/logging-rfc-reference/**`
- Create: `.claude/skills/modern-c23/**`
- Create: `.claude/skills/iojournal-repository-conventions/**`

- [ ] **Step 1: Initialize each skill folder**

Run: `init_skill.py` for each skill with `references` resources and generated interface metadata.
Expected: each skill has `SKILL.md` and `agents/openai.yaml`.

- [ ] **Step 2: Confirm the generated structure**

Run: `find .claude/skills -maxdepth 3 -type f | sort`
Expected: all skill folders are present and the old flat notes are ready to be replaced.

### Task 4: Fill the skills from `docs/tmp`

**Files:**
- Modify: `.claude/skills/iojournal-architecture/SKILL.md`
- Modify: `.claude/skills/iojournal-architecture/references/*`
- Modify: `.claude/skills/iojournal-coding-standards/SKILL.md`
- Modify: `.claude/skills/iojournal-coding-standards/references/*`
- Modify: `.claude/skills/logging-rfc-reference/SKILL.md`
- Modify: `.claude/skills/logging-rfc-reference/references/*`
- Modify: `.claude/skills/modern-c23/SKILL.md`
- Modify: `.claude/skills/modern-c23/references/*`
- Modify: `.claude/skills/iojournal-repository-conventions/SKILL.md`
- Modify: `.claude/skills/iojournal-repository-conventions/references/*`
- Modify: `.claude/skills/ROADMAP.md`

- [ ] **Step 1: Write compact trigger-focused `SKILL.md` files**

Expected: each skill frontmatter explains what it does and when to use it.

- [ ] **Step 2: Move detailed material into `references/`**

Expected: `SKILL.md` stays short, while detailed rules, standards, and checklists live in referenced files.

- [ ] **Step 3: Replace or remove the old flat markdown notes**

Expected: no duplicated topic remains in both old and new formats.

- [ ] **Step 4: Verify the roadmap reflects the new skills**

Expected: `.claude/skills/ROADMAP.md` points to the new folder-based skills.

## Chunk 3: Validation

### Task 5: Validate skills and repository consistency

**Files:**
- Test: `.claude/skills/**`
- Test: `.github/**`
- Test: root instructions and scripts

- [ ] **Step 1: Run the skill validator on every new skill**

Run: `quick_validate.py` for each created skill
Expected: validator exits successfully for all skills.

- [ ] **Step 2: Run repository-level consistency checks**

Run: `find .github .claude/skills scripts -maxdepth 4 -type f | sort`
Expected: confirms the final tree is complete and free of missing expected files.

- [ ] **Step 3: Grep for stale parser identifiers**

Run: `rg -n 'iohttpparser|ihtp_|IHTP_|HTTP parser|scanner -> parser -> semantics' .github .claude/skills AGENTS.md CLAUDE.md CODEX.md scripts`
Expected: no results.

- [ ] **Step 4: Report any remaining non-runnable workflow risks**

Expected: note missing GitHub initialization, missing secrets, or git-dependent release behavior if still present.
