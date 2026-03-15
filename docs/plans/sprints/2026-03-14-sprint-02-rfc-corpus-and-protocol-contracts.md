# Sprint 02: RFC Corpus And Protocol Contracts Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** build the canonical standards corpus and protocol contract set that the RC implementation must follow.

**Architecture:** convert raw RFC research into repository-local contract documents. This sprint should produce navigation, priority, mapping, and divergence docs, not implementation code.

**Tech Stack:** Markdown, local RFC mirror, standards matrices.

---

### Task 1: Produce the canonical RFC list

**Files:**
- Create: `docs/rfc/CANONICAL_LIST.md`
- Create: `docs/rfc/FEATURE_MATRIX.md`
- Modify: `docs/rfc/PRIORITY.md`

- [x] **Step 1: Write the failing coverage check**

Run: `rg -n 'RFC 5424|RFC 5426|RFC 6587|RFC 3339|RFC 8259' docs/rfc`
Expected: canonical list and feature matrix missing or incomplete before the change.

- [x] **Step 2: Add canonical RFC list**

Expected: every RC-relevant RFC or standard is listed with scope and priority.

- [x] **Step 3: Add feature mapping**

Expected: `FEATURE_MATRIX.md` maps standard -> feature -> sprint -> artifact.

- [x] **Step 4: Verify the matrix**

Run: `rg -n 'MUST|SHOULD|MAY' docs/rfc/CANONICAL_LIST.md docs/rfc/FEATURE_MATRIX.md`
Expected: priority terms are present and consistent.

### Task 2: Freeze protocol contracts for the RC

**Files:**
- Create: `docs/rfc/TIMESTAMP_POLICY.md`
- Create: `docs/rfc/FIELD_NAMING_POLICY.md`
- Create: `docs/rfc/SYSLOG_CONTRACT.md`
- Create: `docs/rfc/JSON_NDJSON_CONTRACT.md`
- Create: `docs/rfc/REDACTION_POLICY.md`

- [x] **Step 1: Define timestamp and field naming contracts**

Expected: RFC 3339 timestamp policy and ECS/OTel-aware field naming policy are explicit.

- [x] **Step 2: Define syslog and NDJSON contracts**

Expected: RFC 5424, RFC 5426, RFC 6587, JSON, and NDJSON rules are written down with clear RC scope.

- [x] **Step 3: Define redaction baseline**

Expected: default denylist and masking behavior are fixed before implementation starts.

- [x] **Step 4: Verify cross-document references**

Run: `rg -n 'RFC 5424|RFC 5426|RFC 6587|RFC 3339|OWASP|NDJSON' docs/rfc`
Expected: each contract cites the correct governing standard.

### Task 3: Record explicit divergences and deferred standards

**Files:**
- Create: `docs/rfc/DIVERGENCES_AND_DEFERRALS.md`
- Modify: `docs/plans/BACKLOG.md`

- [x] **Step 1: Document deferred standards**

Expected: RFC 5425 and OTLP are recorded as post-RC.

- [x] **Step 2: Document intentional simplifications**

Expected: RC exclusions and compatibility choices are explicit.

- [x] **Step 3: Verify backlog alignment**

Run: `rg -n 'RFC 5425|OTLP|HEC|Elastic' docs/rfc/DIVERGENCES_AND_DEFERRALS.md docs/plans/BACKLOG.md`
Expected: deferred items appear in both the protocol notes and backlog.
