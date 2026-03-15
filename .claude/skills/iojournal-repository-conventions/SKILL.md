---
name: iojournal-repository-conventions
description: Use when editing repository structure, contributor docs, release files, GitHub workflows, or planning new top-level directories in iojournal. Mandatory for `.github/`, root policy files, docs structure changes, and release/documentation automation work.
---

# Iojournal Repository Conventions

## Overview

Use this skill to keep repository structure, documentation layout, and release-facing files aligned with the shared `io*` project baseline without pulling in parser-specific assumptions.

## Repository Baseline

- Keep long-term plans in `docs/plans/`.
- Treat `docs/tmp/` as research and scratch space, not the canonical surface.
- Keep repository-local skills under `.claude/skills/`.
- Prefer the common `io*` project layout for contributor files, CI, and release assets.

## Documentation And Release Rules

- Contributor-facing root files must stay consistent with the repository purpose.
- Release and changelog material should use factual, non-marketing language.
- Prefer predictable file layout over custom top-level directories.

## Workflow

1. Check whether the change touches repository policy, docs layout, or release automation.
2. Keep the file layout aligned with other `io*` repositories unless `iojournal` has a real need to diverge.
3. Treat bootstrap-only files as scaffolding and keep project-specific claims factual.

## References

- `references/repository-baseline.md`
