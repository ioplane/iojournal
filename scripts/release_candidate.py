#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Run and persist the iojournal release-candidate verification evidence."""

from __future__ import annotations

import platform
import subprocess
import sys
from datetime import UTC, datetime
from pathlib import Path


def run_to_file(command: list[str], *, root: Path, output: Path) -> None:
    with output.open("w", encoding="utf-8") as stream:
        subprocess.run(
            command,
            cwd=root,
            check=True,
            stdout=stream,
            stderr=subprocess.STDOUT,
        )


def capture_command(command: list[str], *, root: Path) -> str:
    result = subprocess.run(
        command,
        cwd=root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    return result.stdout.strip() if result.returncode == 0 else "unavailable"


def write_index(path: Path, *, run_id: str, head_short: str) -> None:
    header = "run_id\tgit_head_short\tstatus"
    rows = [line for line in path.read_text(encoding="utf-8").splitlines() if line]
    if not rows or rows[0] != header:
        rows = [header]
    entry = f"{run_id}\t{head_short}\tPASS"
    rows = [line for line in rows if not line.startswith(f"{run_id}\t")]
    rows.append(entry)
    path.write_text("\n".join(rows) + "\n", encoding="utf-8")


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    run_stamp = datetime.now(UTC).strftime("%Y%m%dT%H%M%SZ")
    head_result = subprocess.run(
        ["git", "rev-parse", "--short", "HEAD"],
        cwd=root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
    )
    head_short = head_result.stdout.strip() if head_result.returncode == 0 else "nogit"
    run_id = f"{run_stamp}-{head_short}"
    artifacts = root / "dist" / "release-candidate"
    output_dir = artifacts / "runs" / run_id
    output_dir.mkdir(parents=True, exist_ok=True)

    (output_dir / "run.txt").write_text(
        "\n".join(
            (
                f"run_id\t{run_id}",
                f"git_head_short\t{head_short}",
                f"utc_started\t{datetime.now(UTC).isoformat(timespec='seconds')}",
            )
        )
        + "\n",
        encoding="utf-8",
    )
    (output_dir / "host.txt").write_text(
        f"{platform.platform()}\n---\n{capture_command(['lscpu'], root=root)}\n",
        encoding="utf-8",
    )
    (output_dir / "toolchain.txt").write_text(
        "\n---\n".join(
            (
                capture_command(["clang", "--version"], root=root),
                capture_command(["cmake", "--version"], root=root),
                capture_command(["python3", "--version"], root=root),
            )
        )
        + "\n",
        encoding="utf-8",
    )
    run_to_file(
        [sys.executable, str(root / "scripts" / "release_gate.py")],
        root=root,
        output=output_dir / "release-gate.txt",
    )
    (output_dir / "summary.md").write_text(
        f"""# Release Candidate Verification Summary

Run id: `{run_id}`

## Verification Scope

- release gate
- Sprint 10 functional and performance comparison evidence
- Sprint 11A and Sprint 11B optimization evidence
- Sprint 12 publication checklist and decision surface
- host and toolchain capture
- bootstrap-state artifact generation

## Published Files

- `run.txt`
- `host.txt`
- `toolchain.txt`
- `release-gate.txt`
- `summary.md`

## Evidence References

- `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- `docs/en/05-release-candidate-checklist.md`
- `docs/en/06-rc1-publish-decision.md`

## Result

Local release-candidate checks completed for this run.
""",
        encoding="utf-8",
    )
    index = artifacts / "index.tsv"
    if not index.exists():
        index.write_text("run_id\tgit_head_short\tstatus\n", encoding="utf-8")
    write_index(index, run_id=run_id, head_short=head_short)
    (artifacts / "latest.txt").write_text(f"{run_id}\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
