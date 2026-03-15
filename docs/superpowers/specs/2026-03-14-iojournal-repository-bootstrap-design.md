# iojournal Repository Bootstrap Design

**Goal:** align `iojournal` with the shared `io*` repository baseline by reusing safe infrastructure from `iohttpparser` and rebuilding `iojournal`-specific local skills from `docs/tmp`.

## Scope

- Copy the reusable repository infrastructure from `iohttpparser`:
  - `.github/`
  - `AGENTS.md`
  - `CLAUDE.md`
  - `CODEX.md`
- Adapt all transferred content to `iojournal`.
- Create repository-local skills under `.claude/skills/` in proper `SKILL.md` format.
- Preserve `docs/tmp/` as the source material for skill content, not as the final authority.

## Non-Goals

- Do not introduce parser-specific rules, terminology, scripts, or workflows.
- Do not invent top-level `.agents/` or `.codex/` directories that do not exist in the source repository.
- Do not carry over benchmark, fuzzing, or parser-only helper scripts.
- Do not rewrite unrelated product documentation in this task.

## Transfer Rules

### `.github/`

- Reuse the baseline workflow set from `iohttpparser`.
- Replace all project identifiers:
  - `iohttpparser` -> `iojournal`
  - `ihtp_` / `IHTP_` -> `ij_` / `IJ_`
  - `IOHTTPPARSER_*` -> `IOJOURNAL_*`
- Keep only workflows that can be backed by repository-local scripts after transfer.
- Reword issue and PR templates so they describe `iojournal` as a logging library, not an HTTP parser.

### Root Instructions

- Keep the structure and tone from `iohttpparser`.
- Replace parser-specific guidance with `iojournal` constraints:
  - zero-allocation hot path
  - ring-buffer architecture
  - sinks/encoders/filter boundaries
  - redaction and observability rules
- Keep the shared utility/tooling guidance when still relevant.

### CI/Release Scripts

- Transfer only the scripts required by reused workflows.
- Prefer the minimal set:
  - `scripts/quality.sh`
  - `scripts/lint-docs.py`
  - `scripts/run-release-gate.sh`
  - `scripts/run-coverage.sh`
  - `scripts/run-release-candidate.sh`
  - `scripts/build-release-assets.sh`
  - `scripts/render-release-notes.sh`
- Audit transferred scripts for parser-specific assumptions and replace them with `iojournal` build targets, options, and artifact names.

## Local Skills Design

Repository-local skills will use the standard structure:

```text
.claude/skills/<skill-name>/
  SKILL.md
  agents/openai.yaml
  references/...
```

### Skills To Create

1. `iojournal-architecture`
   - boundaries between core, encoders, sinks, filters, and context/runtime pieces
   - hot-path invariants and backpressure model
2. `iojournal-coding-standards`
   - naming, public/internal API rules, test expectations, safety rules
3. `logging-rfc-reference`
   - syslog, OTLP, NDJSON, timestamps, OWASP redaction guidance
4. `modern-c23`
   - C23 features approved for `iojournal`
5. `iojournal-repository-conventions`
   - repository structure, docs style, changelog/release expectations, tool baseline

### Source Material Mapping

- `docs/tmp/draft/iojournal.md`
  - product and architecture direction
- `docs/tmp/draft/kimi_iojournal/iojournal_guide.md`
  - architecture and implementation checklist
- `docs/tmp/draft/kimi_iojournal/logging_rfcs_standards.md`
  - standards reference
- `docs/tmp/draft/c23-best-practices.md`
  - C23 references
- `docs/tmp/draft/kimi_iojournal/c23_logging_library_features.md`
  - C23 logging-oriented guidance
- `docs/tmp/draft/kimi_iojournal/c_logging_library_design_patterns.md`
  - queueing and async patterns
- `docs/tmp/draft/kimi_iojournal/c_logging_libraries_analysis.md`
  - comparison material for references
- `docs/tmp/draft/git-project-recommendations-for-c23.md`
  - repository/documentation conventions

## Migration Of Existing `.claude/skills/*`

- Replace the current flat markdown notes with skill folders where the topic survives.
- Keep `ROADMAP.md` as a lightweight index if it still reflects the new skill set.
- Do not leave duplicate content in both the old flat files and the new references.

## Validation

- Confirm all transferred workflow references point to files that exist in `iojournal`.
- Validate every new skill with the skill-creator validator.
- Verify the final tree contains:
  - adapted `.github/`
  - adapted root instructions
  - required CI/release scripts
  - the new `.claude/skills/<skill>/...` structure

## Constraints

- The repository is not yet initialized on GitHub, and git integration may be incomplete locally.
- Because of that, the task focuses on filesystem state and local consistency, not on pushing, tagging, or opening PRs.
