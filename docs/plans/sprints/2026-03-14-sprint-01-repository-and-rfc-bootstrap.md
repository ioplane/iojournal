# Sprint 01: Repository And RFC Bootstrap Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** create the repository, planning, and standards-ingestion baseline needed before protocol and implementation work can start.

**Architecture:** keep this sprint documentation-heavy and infrastructure-light. Build only the minimum scripts and docs surfaces needed to support later RFC and implementation work.

**Tech Stack:** Markdown, Bash, Python repository helpers, docs indexes, local RFC mirror.

**Execution Note:** verification for this sprint runs inside `localhost/iojournal-dev:latest`. Stable docs for this sprint now include `docs/en/01-bootstrap-and-rfc-corpus.md` and `docs/ru/01-bootstrap-and-rfc-corpus.md`.

---

### Task 1: Stabilize the planning and docs surface

**Files:**
- Create: `docs/README.md`
- Create: `docs/en/README.md`
- Create: `docs/ru/README.md`
- Create: `docs/en/01-bootstrap-and-rfc-corpus.md`
- Create: `docs/ru/01-bootstrap-and-rfc-corpus.md`
- Modify: `docs/plans/README.md`
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/BACKLOG.md`
- Modify: `CHANGELOG.md`

- [x] **Step 1: Write the failing structure check**

Run: `find docs -maxdepth 2 -type f | sort`
Expected: missing stable docs indexes before the change.

- [x] **Step 2: Add docs indexes, planning links, and the first stable numbered docs**

Expected: docs indexes exist and point to plans, RFC mirror, future stable docs, and the Sprint 01 bootstrap contract.

- [x] **Step 3: Update changelog for planning baseline**

Expected: `CHANGELOG.md` records the bootstrap planning surface.

- [x] **Step 4: Verify the docs structure in the container**

Run:
- `find docs -maxdepth 2 -type f | sort`
- `podman run --rm -v /opt/projects/repositories/iojournal:/workspace:Z -w /workspace localhost/iojournal-dev:latest python3 scripts/lint-docs.py`

Expected: docs indexes, plan files, and stable numbered docs are present and pass docs lint.

### Task 2: Prepare RFC harvesting and source catalog

**Files:**
- Modify: `scripts/rfc-scraper.py`
- Create: `docs/rfc/README.md`
- Create: `docs/rfc/SOURCES.md`
- Create: `docs/rfc/PRIORITY.md`
- Create: `tests/unit/test_rfc_scraper.py`

- [x] **Step 1: Write the failing discovery check**

Run: `find docs/rfc -maxdepth 1 -type f | sort`
Expected: no source catalog or priority document yet.

- [x] **Step 2: Add RFC source catalog**

Expected: `SOURCES.md` lists official sources, retrieval policy, and target RFC set.

- [x] **Step 3: Add RFC priority document**

Expected: `PRIORITY.md` classifies MUST/SHOULD/MAY references for the RC.

- [x] **Step 4: Make scraper usage align with the catalog and container workflow**

Run:
- `podman run --rm -v /opt/projects/repositories/iojournal:/workspace:Z -w /workspace localhost/iojournal-dev:latest python3 -m unittest tests/unit/test_rfc_scraper.py`
- `podman run --rm -v /opt/projects/repositories/iojournal:/workspace:Z -w /workspace localhost/iojournal-dev:latest python3 scripts/rfc-scraper.py -o docs/rfc/registry.md`

Expected: `scripts/rfc-scraper.py` uses supported Datatracker filters, tests pass, and docs explain the Podman execution path.

### Task 3: Freeze RC non-goals and dependency map

**Files:**
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/BACKLOG.md`
- Modify: `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`

- [x] **Step 1: Record explicit non-goals**

Expected: OTLP, TLS syslog, HEC, Elastic, SHM, spooling, and encryption-at-rest are out of scope for RC.

- [x] **Step 2: Record dependency order**

Expected: later sprints depend on standards and contracts from Sprints 01-04.

- [x] **Step 3: Verify planning consistency**

Run: `rg -n 'v0.1.0-rc.1|OTLP|RFC 5424|RFC 5426|RFC 6587' docs/plans docs/rfc`
Expected: scope and dependency terms are represented consistently.
