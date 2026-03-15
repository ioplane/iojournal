# iojournal v0.1.0-rc.1 Master Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** take `iojournal` from repository bootstrap to the first public `v0.1.0-rc.1`.

**Architecture:** execute a protocol-first program. Standards and contracts land before core runtime work; runtime work lands before persistence and syslog delivery; release evidence lands only after runnable features exist.

**Tech Stack:** Markdown planning docs, local RFC mirror, Bash/Python repository scripts, future C23/CMake implementation surface, GitHub Actions bootstrap.

---

## Program Map

| Sprint | Main Deliverables | Exit Artifact |
|---|---|---|
| 01 | docs baseline, Python RFC harvesting procedure, initial standards list | docs structure + source catalog |
| 02 | canonical RFC list, priority map, field naming and severity rules | standards contract pack |
| 03 | public API draft, event model, ownership/error contracts | API contract pack |
| 04 | queueing model, overflow rules, stress matrix | concurrency contract pack |
| 05 | core runtime MVP, JSON console sink, unit-test skeleton | runnable core build |
| 06 | NDJSON file sink, rotation/retention baseline | local persistence build |
| 07 | RFC 5424 formatter, RFC 5426/6587 sinks, interoperability fixtures | syslog-capable RC candidate |
| 08 | examples, release evidence, packaging, RC checklist | `v0.1.0-rc.1` publish decision |

## Standards Matrix

| Standard / Reference | Scope | Priority | First Sprint Using It | RC Requirement |
|---|---|---|---|---|
| RFC 5424 | syslog format and structured data | MUST | 02 | yes |
| RFC 5426 | UDP syslog transport | MUST | 02 | yes |
| RFC 6587 | TCP syslog framing | MUST | 02 | yes |
| RFC 3339 | timestamp profile | MUST | 02 | yes |
| RFC 8259 | JSON syntax | MUST | 02 | yes |
| NDJSON conventions | file and bulk line format | MUST | 02 | yes |
| OWASP Logging Cheat Sheet | redaction and exclusions | MUST | 02 | yes |
| Elastic Common Schema | field naming guidance | SHOULD | 02 | partial |
| OpenTelemetry log model | future exporter/trace alignment | SHOULD | 02 | partial |
| RFC 5425 | syslog over TLS | MAY for RC, deferred | 02 | no |

## RFC Handoff Matrix

| Standard / Reference | Sprint 01 Artifact | Sprint 02 Decision | First Implementation Sprint | RC Status |
|---|---|---|---|---|
| RFC 5424 | `docs/rfc/registry.md`, `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | freeze field mapping and structured-data scope | 07 | required |
| RFC 5426 | `docs/rfc/registry.md`, `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | freeze UDP transport assumptions | 07 | required |
| RFC 6587 | `docs/rfc/registry.md`, `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | freeze TCP framing rules | 07 | required |
| RFC 3339 | `docs/rfc/registry.md`, `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | freeze timestamp profile | 03 | required |
| RFC 8259 | `docs/rfc/registry.md`, `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | freeze JSON encoding constraints | 05 | required |
| NDJSON conventions | `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | freeze line-oriented file format rules | 06 | required |
| OWASP Logging Cheat Sheet | `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | freeze exclusion and redaction baseline | 03 | required |
| Elastic Common Schema | `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | decide minimum field naming subset | 03 | partial |
| OpenTelemetry log model | `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | decide alignment boundaries only | 03 | partial |
| RFC 5425 | `docs/rfc/registry.md`, `docs/rfc/SOURCES.md`, `docs/rfc/PRIORITY.md` | record as deferred transport-security work | post-RC | deferred |

## RC Non-Goal Freeze

| Deferred Item | Why out of scope for `v0.1.0-rc.1` | First reconsideration point |
|---|---|---|
| RFC 5425 syslog over TLS | requires transport security and certificate lifecycle contracts not covered by Sprint 07 | post-RC protocol expansion |
| OTLP/HTTP exporter | requires a second protocol family and exporter schema beyond the RC sink set | post-RC exporter work |
| Splunk HEC and Elastic Bulk | vendor-specific delivery semantics are outside the first public RC | post-RC exporter work |
| shared-memory bus and spool queues | require new runtime and persistence guarantees beyond Sprints 04-06 | post-RC runtime work |
| encryption at rest | requires key-management and recovery policy outside the RC persistence baseline | post-RC persistence work |

## Sprint 01 Outputs

- docs indexes under `docs/`, `docs/en/`, and `docs/ru/`
- RFC navigation docs under `docs/rfc/`
- documented Python workflow for `scripts/rfc-scraper.py`
- explicit RC non-goals and deferred standards backlog

## Chunk 1: Program Initialization

### Task 1: Establish the planning baseline

**Files:**
- Modify: `docs/plans/README.md`
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/BACKLOG.md`
- Modify: `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
- Test: `docs/plans/sprints/README.md`

- [ ] **Step 1: Publish the authoritative plan index**

Run: `find docs/plans -maxdepth 2 -type f | sort`
Expected: roadmap, backlog, master plan, and sprint plans are present.

- [ ] **Step 2: Freeze the RC scope**

Expected decisions:
- console sink in RC
- file sink in RC
- syslog UDP/TCP in RC
- OTLP/TLS/HEC/Elastic deferred

- [ ] **Step 3: Freeze sprint sequencing**

Expected: eight named sprints with explicit dependencies.

## Chunk 2: Sprint Execution Handoff

### Task 2: Prepare the sprint plan set

**Files:**
- Modify: `docs/plans/sprints/README.md`
- Modify: `docs/plans/sprints/2026-03-14-sprint-01-repository-and-rfc-bootstrap.md`
- Modify: `docs/plans/sprints/2026-03-14-sprint-02-rfc-corpus-and-protocol-contracts.md`
- Modify: `docs/plans/sprints/2026-03-14-sprint-03-public-api-and-data-model.md`
- Modify: `docs/plans/sprints/2026-03-14-sprint-04-buffering-and-concurrency-spec.md`
- Modify: `docs/plans/sprints/2026-03-14-sprint-05-core-runtime-mvp.md`
- Modify: `docs/plans/sprints/2026-03-14-sprint-06-file-sink-and-persistence.md`
- Modify: `docs/plans/sprints/2026-03-14-sprint-07-syslog-protocol-delivery.md`
- Modify: `docs/plans/sprints/2026-03-14-sprint-08-release-candidate-hardening.md`

- [ ] **Step 1: Keep each sprint isolated**

Expected: each sprint file has local goals, files, commands, and exit evidence.

- [ ] **Step 2: Reference upstream decisions instead of duplicating them**

Expected: sprint plans point back to roadmap/master plan for scope and dependency context.

- [ ] **Step 3: Keep post-RC work out of sprint execution**

Expected: backlog items do not leak into the eight RC sprints.

## Execution Notes

- Execute Sprint 01 and Sprint 02 before starting any feature code.
- Treat Sprint 03 and Sprint 04 as contract freeze gates.
- Do not claim RC readiness before Sprint 08 verification evidence exists.
- Do not promote backlog items into Sprint 05-08 unless the roadmap and standards matrices are intentionally revised first.
