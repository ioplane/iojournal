# iojournal Plans

This directory is the authoritative planning surface for `iojournal`.

## Files

- `ROADMAP.md`
  high-level program from zero-state repository to `v0.1.0-rc.1`
- `BACKLOG.md`
  deferred work that is intentionally out of scope for `v0.1.0-rc.1`
- `2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
  cross-cutting master plan with dependencies, deliverables, and RFC mapping
- `2026-03-16-iojournal-functional-and-performance-comparison-plan.md`
  comparison and benchmark program for release-blocking functional/performance evidence
- `../superpowers/specs/2026-03-16-iojournal-bottleneck-and-optimization-design.md`
  validated design for the release-blocking bottleneck, `io_uring`, and SIMD optimization sprint
- `sprints/2026-03-16-sprint-11b-event-copy-and-allocation-optimization.md`
  release-blocking follow-on sprint for event-copy and allocation-churn reduction
- `sprints/2026-03-16-sprint-12-final-rc-decision-and-publication.md`
  publication hardening sprint and explicit `v0.1.0-rc.1` decision record
- `comparison/`
  release-blocking functional/performance evidence artifacts for Sprint 10
- `sprints/`
  detailed sprint plans in execution-ready format

## Planning Rules

- Treat `ROADMAP.md` as the top-level sequence of record.
- Treat the master plan as the cross-sprint dependency map.
- Treat each sprint file as the execution plan for that sprint.
- Update the roadmap and affected sprint plans in the same branch when scope changes.
- Keep `docs/tmp/` as research input only; copy validated decisions here before execution.
- Record the current publish or no-publish outcome in the stable decision docs under `docs/en/` and `docs/ru/`.

## Mandatory Update Protocol

- Treat this file set as the minimal source of truth for planning handoffs:
  - `docs/plans/ROADMAP.md`
  - `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`
  - `docs/plans/sprints/README.md`
  - active sprint file under `docs/plans/sprints/`
- If publish status changes, update in one transaction:
  - `docs/en/06-rc1-publish-decision.md`
  - `docs/ru/06-rc1-publish-decision.md`
  - `docs/en/05-release-candidate-checklist.md`
  - `docs/ru/05-release-candidate-checklist.md`
- Always run after plan or evidence changes:
  - `python3 scripts/lint-docs.py`
  - `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest python scripts/quality.py`
- For perf/comparison scope changes, also keep these files synchronized in the same commit:
  - `docs/plans/comparison/PERFORMANCE_RESULTS.md`
  - `docs/plans/comparison/RAW_ARTIFACTS.md`
  - `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`
  - `docs/testing/IO_URING_RELEVANCE.md`
  - `docs/testing/SIMD_RELEVANCE.md`
