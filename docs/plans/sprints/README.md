# iojournal Sprint Plans

These files are the detailed execution plans for the `v0.1.0-rc.1` program.

## Order

1. `2026-03-14-sprint-01-repository-and-rfc-bootstrap.md`
2. `2026-03-14-sprint-02-rfc-corpus-and-protocol-contracts.md`
3. `2026-03-14-sprint-03-public-api-and-data-model.md`
4. `2026-03-14-sprint-04-buffering-and-concurrency-spec.md`
5. `2026-03-14-sprint-05-core-runtime-mvp.md`
6. `2026-03-14-sprint-06-file-sink-and-persistence.md`
7. `2026-03-14-sprint-07-syslog-protocol-delivery.md`
8. `2026-03-14-sprint-08-release-candidate-hardening.md`
9. `2026-03-16-sprint-09-benchmark-harness-and-methodology.md`
10. `2026-03-16-sprint-10-functional-and-performance-comparison.md`
11. `2026-03-16-sprint-11a-bottleneck-and-performance-optimization.md`
12. `2026-03-16-sprint-11b-event-copy-and-allocation-optimization.md`
13. `2026-03-16-sprint-11c-structural-hot-path-redesign.md`
14. `2026-03-16-sprint-11d-contract-preserving-fast-paths.md`
15. `2026-03-16-sprint-12-final-rc-decision-and-publication.md`
16. `2026-03-17-sprint-13-audit-bug-fixes-and-test-hardening.md`
17. `2026-03-17-sprint-14-shell-script-modernization.md`

## Usage

- Run sprints in order unless the roadmap is explicitly revised.
- Treat each sprint file as the execution source of truth for that sprint.
- Update the roadmap and master plan when a sprint changes scope.
- Before closing or handing off any sprint, re-read:
  - `ROADMAP.md`
  - `sprints/<active>.md`
  - `docs/en/06-rc1-publish-decision.md`
  - `docs/ru/06-rc1-publish-decision.md` (if publish status changed)
- For handoff, keep in the active sprint file: current status, evidence source-of-record IDs, next three priority commands, and open blockers.
- For each completed sprint task, run mandatory checks before commit:
  - `python3 scripts/lint-docs.py`
  - `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest python scripts/quality.py`
- For sprint changes that touch evidence or release gates, additionally run:
  - `uv run --script scripts/release_candidate.py`
  - `uv run --script scripts/release_assets.py v0.1.0-rc.1`
  - `uv run --script scripts/release_notes.py v0.1.0-rc.1`
- Sprint 03 produces the RC API contract pack under `docs/api/`.
- Sprint 04 produces the RC concurrency contract pack under `docs/architecture/` and `docs/testing/`.
- Sprint 09 produces the benchmark and profiler methodology surface.
- Sprint 10 produces the release-blocking comparison evidence pack.
- Sprint 11A produces the release-blocking bottleneck and optimization evidence pack.
- Sprint 11B is complete and hands off to the next optimization tranche.
- Sprint 11C is complete and hands off to the next optimization tranche.
- Sprint 11D is complete and passed evidence refresh.
- Sprint 12 Task 0 is complete; publication hardening continues pending remote gate.
- Sprint 13 is complete: 6 bug fixes (2 CRITICAL, 4 HIGH), 33 new tests, quality gate PASS 14/14.
- Sprint 14 is complete: 14 scripts rewritten, shared library created, shellcheck gate added, quality gate PASS 15/15.
