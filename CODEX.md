# iojournal - Project Instructions for Codex

## Start Here

Read these in order before making non-trivial changes:
- `AGENTS.md`
- `CLAUDE.md`
- `docs/plans/2026-03-10-iojournal-c23-architecture-plan.md`
- `.claude/skills/ROADMAP.md`

## Core Working Rules

- Preserve the public API shape in `include/iojournal/` unless the task explicitly requires an API change.
- Keep the logging pipeline responsibilities narrow: event capture, bounded buffering, filtering, encoding, and sink delivery.
- Do not move application routing, HTTP parsing, OpenAPI validation, or unrelated transport logic into `iojournal`.
- Default to safe behavior. Any lenient or lossier behavior must be explicit, documented, and covered by tests.
- Keep the hot path allocation-free and avoid hidden unbounded work.

## Code and Review Focus

- Public names: `ij_*`
- Pipeline ownership: producer -> buffer -> filter -> encoder -> sink
- Security focus: redaction of tokens, secrets, cookies, passwords, and session identifiers
- Observability focus: RFC 5424, RFC 5425/5426/6587, NDJSON, OTLP compatibility
- Performance focus: bounded queues, backpressure policy, `io_uring`-aware sinks when applicable

## Required Follow-Through

- Update unit tests when behavior changes.
- Update docs when layer boundaries, defaults, or external contracts change.
- Keep `CLAUDE.md`, `CODEX.md`, and `.claude/skills/` aligned when repository rules change.

## Local Skills

Repository-local skills live under `.claude/skills/`. They are the closest thing to project memory for repeated architecture, standards, and repository decisions, even when the active agent is not Claude.

## Required Utilities

For effective Codex work on this repository, keep these available on the host:
- `git` for branches, worktrees, history, and patch-oriented review
- `gh` with working auth, especially `gh api graphql`, for repository and PR automation
- `rg` (`ripgrep`) for fast code and path discovery
- `jq` for processing JSON output from GitHub APIs, tool output, and generated reports
- `python3` for project-local automation and validation scripts
- `podman` for the development and quality execution environment
- `uv` / `uvx` for optional MCP and helper tooling such as Serena
- `clangd` for semantic C/C++ navigation when LSP-based tooling is used

Useful but optional:
- `fd` for fast filename discovery
- `yq` for YAML inspection
- `hyperfine` for repeatable benchmark comparisons
