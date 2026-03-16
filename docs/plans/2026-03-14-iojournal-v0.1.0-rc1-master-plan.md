# iojournal v0.1.0-rc.1 Master Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** take `iojournal` from repository bootstrap to the first public `v0.1.0-rc.1` with mandatory feature, functional-comparison, performance-comparison, and optimization evidence.

**Architecture:** execute a protocol-first program. Standards and contracts land before core runtime work; runtime work lands before persistence and syslog delivery; release infrastructure lands before benchmark methodology; benchmark methodology lands before competitor comparison; competitor comparison lands before the bottleneck and optimization sprint; publish evidence lands only after runnable features, comparison evidence, and optimization evidence exist.

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
| 08 | release scripts, examples, packaging, RC checklist scaffolding | RC infrastructure ready |
| 09 | benchmark harness, profiling stack, methodology, raw artifact flow | benchmark-comparison harness ready |
| 10 | Tier 1 functional and performance comparison, Tier 2 appendix | comparison evidence pack under `docs/plans/comparison/` |
| 11A | bottleneck inventory, `io_uring` relevance and adoption decision, SIMD relevance and adoption decision, refreshed comparison evidence | optimization evidence pack |
| 11B | event-copy and allocation-churn optimization, refreshed benchmark and profiler evidence | allocation-optimization evidence pack |
| 11C | structural hot-path redesign, refreshed benchmark and profiler evidence | structural-optimization evidence pack |
| 11D | contract-preserving fast paths, refreshed benchmark and profiler evidence | fast-path optimization evidence pack |
| 12 | final publish criteria, evidence review, tag/release prep | `v0.1.0-rc.1` publish decision |

## Current Program Status

- Sprint 01 through Sprint 11C are complete.
- Sprint 11D is complete and active. The optimization evidence pack is:
  - local benchmark run [`20260316-210134`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134)
  - Tier 1 benchmark run [`20260316-150320`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320)
  - callgrind and repeatability pack [`20260316-210204`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204)
  - `uftrace` companion runs [`20260316-210329`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210329), [`20260316-210345`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210345), and [`20260316-210349`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210349)
- Sprint 12 publication is now the release-gate sprint.
- The current explicit publication decision remains `NO-PUBLISH`; see [`docs/en/06-rc1-publish-decision.md`](/opt/projects/repositories/iojournal/docs/en/06-rc1-publish-decision.md).

## Sprint 10 Evidence Pack

| Artifact | Role | Release Status |
|---|---|---|
| `docs/plans/comparison/TIER1_FUNCTIONAL_MATRIX.md` | normalized Tier 1 capability matrix | required |
| `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md` | consumer-fit decision view for `iohttp` and `ioguard` | required |
| `docs/plans/comparison/TIER2_APPENDIX.md` | reference-only ecosystem context | required |
| `docs/plans/comparison/PERFORMANCE_RESULTS.md` | normalized shared and capability-specific results | required |
| `docs/plans/comparison/RAW_ARTIFACTS.md` | benchmark and profiling artifact registry | required |

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
| `zlog_alternative.md` | C logging ecosystem shortlist and suitability hints | MUST for comparison program | 09 | yes |

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
| `docs/tmp/draft/zlog_alternative.md` | research input only | freeze Tier 1 and Tier 2 comparison sets | 09 | required as research input |

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

Expected: eleven named sprints with explicit dependencies.

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
- Create: `docs/plans/sprints/2026-03-16-sprint-09-benchmark-harness-and-methodology.md`
- Create: `docs/plans/sprints/2026-03-16-sprint-10-functional-and-performance-comparison.md`
- Create: `docs/plans/sprints/2026-03-16-sprint-11-final-rc-decision-and-publication.md`

- [ ] **Step 1: Keep each sprint isolated**

Expected: each sprint file has local goals, files, commands, and exit evidence.

- [ ] **Step 2: Reference upstream decisions instead of duplicating them**

Expected: sprint plans point back to roadmap/master plan for scope and dependency context.

- [ ] **Step 3: Keep post-RC work out of sprint execution**

Expected: backlog items do not leak into the eleven RC sprints.

## Execution Notes

- Execute Sprint 01 and Sprint 02 before starting any feature code.
- Treat Sprint 03 and Sprint 04 as contract freeze gates.
- Do not claim RC readiness before Sprint 11 verification evidence exists.
- Do not promote backlog items into Sprint 05-11 unless the roadmap and standards matrices are intentionally revised first.
