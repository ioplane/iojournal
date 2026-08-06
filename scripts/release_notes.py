#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Render deterministic iojournal release notes."""

from __future__ import annotations

import os
import sys
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    tag = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("GITHUB_REF_NAME", "dev")
    latest = root / "dist/release-candidate/latest.txt"
    rc_run_id = (
        latest.read_text(encoding="utf-8").strip()
        if latest.is_file()
        else "bootstrap-not-run"
    )
    (root / "dist").mkdir(parents=True, exist_ok=True)
    (root / "dist/RELEASE_NOTES.md").write_text(
        f"""# iojournal {tag}

## Scope

- bounded logging library for the `io*` ecosystem
- synchronous RC API with stable console, file, and syslog sinks
- release-blocking comparison and optimization evidence included in the RC decision surface

## Verification

- release gate: `scripts/release_gate.py`
- release-candidate evidence: `{rc_run_id}`
- comparison evidence: `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- publish decision: `docs/en/06-rc1-publish-decision.md`

## Published Assets

- source tarball
- source zip archive when `zip` is available
- generated API reference archive when docs have been built
- verification artifact archive when release-candidate output exists
- checksums

## References

- roadmap: `docs/plans/ROADMAP.md`
- release checklist: `docs/en/05-release-candidate-checklist.md`
- publish decision: `docs/en/06-rc1-publish-decision.md`
- local RFC mirror: `docs/rfc/`
""",
        encoding="utf-8",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
