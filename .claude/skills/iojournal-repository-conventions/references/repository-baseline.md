# iojournal Repository Baseline

## Canonical Layout

- `docs/plans/` for plans and roadmaps
- `docs/tmp/` for research and drafts only
- `.claude/skills/` for repository-local skills
- `deploy/podman/` for the development container
- root policy files for contributors and agents

## Change Rules

- Keep repository structure aligned with other `io*` projects unless divergence is justified.
- Do not treat draft docs as canonical requirements without promoting them deliberately.
- Prefer factual contributor and release language over marketing wording.
- Keep workflow and release metadata specific to `iojournal`, not inherited parser terminology.

## Typical Triggers

- editing `.github/`
- changing `AGENTS.md`, `CLAUDE.md`, or `CODEX.md`
- introducing new top-level directories
- changing docs layout or release artifacts
