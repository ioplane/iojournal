[![GitHub](https://img.shields.io/badge/GitHub-iojournal-181717)](https://github.com/ioplane/iojournal)

# iojournal Agent Development Plan

Date: 2026-10-26. Audited default branch: `main` at
`78bc113ad6240abac193e0860b24f44a320d8cb4`. Planning audit: `ngtun-i8s.42.2`.
Durable execution work order: `ngtun-4zy.3` under independent portfolio `ngtun-4zy`.

Historical plans and local uncommitted August plans remain preserved. This dated
plan supplies the next validation/implementation contracts; it does not rewrite past
measurements, sprint completion records or current publish decisions.

## Verified baseline

The default tree contains synchronous bounded logging, console/NDJSON and UDP/TCP syslog
surfaces with redaction. `docs/plans/ROADMAP.md:78-88` records completed historical sprints
and a NO-PUBLISH decision. `docs/en/06-rc1-publish-decision.md:9-14` retains NO-PUBLISH;
its old local workspace and workflow observations are dated evidence. GitHub check jobs
67430002209 (release-gate) and 67429999934 (coverage) report failure at this base revision;
this audit did not diagnose or rerun them.

## Next functional slice

Resolve the existing PR #3 tranche and produce one exact-revision RC decision. Preserve
byte output, redaction, event ownership, queue bounds and shutdown/flush behavior while
checking the proposed optimizations. Include sink failure/rotation, queue-full, bounded
backpressure and flush/shutdown controls; record actual allocation behavior. Refresh the relevant profiler/comparison evidence
only after correctness and the required container quality gate pass. This plan does not
change historical sprint order, sink scope or the NO-PUBLISH decision.

| Order | Owned paths | Exit evidence |
| --- | --- | --- |
| 1 | PR #3's existing runtime/quality/doc changes | Review current patchset in place; equivalence, malformed UTF-8/redaction, lifetime and concurrency controls |
| 2 | Existing release/coverage workflows and candidate harness | Fresh exact-SHA required remote gates; failures have cause and scoped regression |
| 3 | Existing EN/RU decision/checklist and artifact registry | Verified hashes and explicit publish/no-publish decision for one source/profile |

## Dependencies and overlap

PR #3 owns `ij_event.c`, `ij_json_console.c`, `ij_redact.c`,
`ij_redact_match_scalar.c`, controllers, ROADMAP and publish-decision docs. No competing
optimization, controller migration or status rewrite is assigned elsewhere. iohttp/ioguard
integration requires a separate bounded consumer adapter and backpressure/redaction proof;
source benchmarks alone do not establish application fit. ngtun is not a logging runtime
dependency. OTLP, TLS syslog, new queue topology and spool persistence remain outside RC.

## Publication contract

Retain `docs/plans/ROADMAP.md` as sequence of record. Sequence changes also update the
master plan, sprint index and active sprint; publish-status changes synchronously update
both EN/RU decision and checklist files. Performance changes keep comparison, raw-artifact,
fit and IO_URING/SIMD relevance docs aligned. Use the existing perf-lane wrapper only for
its authorized profile; ordinary checks stay on the normal container lane. CHANGELOG and
full container quality/release evidence are required before delivery. No historical
microbenchmark establishes current throughput or permits publication.

## Execution and evidence rules

Read the target repository contract and applicable local skills before each leaf.
This independent portfolio does not gate ngtun release or require ngtun adoption.
Use an actually qualified project OCI; shared-base publication gates only its adoption.
Preserve dirty worktrees, unpublished plans, existing PRs and their path ownership.
Refresh exact default/PR identities before execution; keep compatibility explicit.
Record actual nonempty test/analyzer coverage and exact source/tool/image evidence.
Follow the repository task, changelog, review and delivery contract for each leaf.
This planning update claims no runtime success and authorizes no live mutation.
Repository quality gates remain pending until run; no host build or engine was used.

## Source baseline

- [Default README](https://github.com/ioplane/iojournal/blob/78bc113ad6240abac193e0860b24f44a320d8cb4/README.md).
- [Default repository contract](https://github.com/ioplane/iojournal/blob/78bc113ad6240abac193e0860b24f44a320d8cb4/AGENTS.md); applicable dirty local contracts were read without importing their changes.
- [Existing PR #3](https://github.com/ioplane/iojournal/pull/3), audited head `13d1f8f03b5ebf5f86df465183d019ae68db5605`; recheck the patchset before execution.
- [docs/plans/ROADMAP.md:78](https://github.com/ioplane/iojournal/blob/78bc113ad6240abac193e0860b24f44a320d8cb4/docs/plans/ROADMAP.md#L78).
- [docs/en/06-rc1-publish-decision.md:9](https://github.com/ioplane/iojournal/blob/78bc113ad6240abac193e0860b24f44a320d8cb4/docs/en/06-rc1-publish-decision.md#L9).
