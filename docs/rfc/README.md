# iojournal RFC And Standards Corpus

## Purpose

This directory stores the local RFC mirror and the navigation documents that define the standards baseline for `iojournal`.

## Files

- `registry.md`
  generated RFC registry produced by the Python scraper
- `CANONICAL_LIST.md`
  frozen RC target set with scope and status per reference
- `FEATURE_MATRIX.md`
  mapping from reference to feature surface, sprint, and artifact
- `SOURCES.md`
  authoritative source list and retrieval policy
- `PRIORITY.md`
  MUST/SHOULD/MAY classification for the release candidate
- `TIMESTAMP_POLICY.md`
  canonical timestamp serialization contract
- `FIELD_NAMING_POLICY.md`
  library-native field naming contract
- `JSON_NDJSON_CONTRACT.md`
  JSON and NDJSON emission rules
- `REDACTION_POLICY.md`
  default redaction rules and denylist boundary
- `SYSLOG_CONTRACT.md`
  RFC 5424, RFC 5426, and RFC 6587 RC contract
- `DIVERGENCES_AND_DEFERRALS.md`
  intentional standards exclusions and simplifications for the RC
- `*.txt`
  local RFC text mirrors

## Ownership

- `registry.md` and `rfc*.txt` are generated or refreshed by containerized Python workflow.
- `SOURCES.md`, `CANONICAL_LIST.md`, `FEATURE_MATRIX.md`, and `PRIORITY.md` are manually curated review documents.
- Stable decisions move from this directory into `docs/en/*`, `docs/ru/*`, and `docs/plans/*`.

## Workflow

1. Run the RFC scraper inside `localhost/iojournal-dev:latest`.
2. Refresh `registry.md` before editing `SOURCES.md`, `CANONICAL_LIST.md`, `FEATURE_MATRIX.md`, or `PRIORITY.md`.
3. Keep curated standards docs under manual review.
4. Promote standards decisions into `docs/plans/` and later into stable docs under `docs/en/` and `docs/ru/`.

## Commands

```bash
podman run --rm -v /opt/projects/repositories/iojournal:/workspace:Z -w /workspace \
  localhost/iojournal-dev:latest python3 scripts/rfc-scraper.py -o docs/rfc/registry.md
podman run --rm -v /opt/projects/repositories/iojournal:/workspace:Z -w /workspace \
  localhost/iojournal-dev:latest python3 scripts/rfc-scraper.py --download docs/rfc
podman run --rm -v /opt/projects/repositories/iojournal:/workspace:Z -w /workspace \
  localhost/iojournal-dev:latest python3 scripts/rfc-scraper.py --download-all docs/rfc
```

## Verification

```bash
podman run --rm -v /opt/projects/repositories/iojournal:/workspace:Z -w /workspace \
  localhost/iojournal-dev:latest python3 -m unittest tests/unit/test_rfc_scraper.py
podman run --rm -v /opt/projects/repositories/iojournal:/workspace:Z -w /workspace \
  localhost/iojournal-dev:latest python3 scripts/lint-docs.py
```
