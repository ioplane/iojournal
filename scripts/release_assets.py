#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Build reproducible iojournal release archives and checksums."""

from __future__ import annotations

import hashlib
import os
import subprocess
import sys
from pathlib import Path


def run(command: list[str], *, root: Path) -> None:
    subprocess.run(command, cwd=root, check=True)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    dist = root / "dist"
    tag = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("GITHUB_REF_NAME", "dev")
    prefix = f"iojournal-{tag}"
    dist.mkdir(parents=True, exist_ok=True)
    for name in (
        f"{prefix}.tar.gz",
        f"{prefix}.zip",
        f"{prefix}-docs.tar.gz",
        f"{prefix}-verification.tar.gz",
        f"{prefix}.sha256",
        "RELEASE_NOTES.md",
    ):
        (dist / name).unlink(missing_ok=True)
    if (root / ".git").exists():
        run(
            [
                "git",
                "archive",
                "--format=tar.gz",
                f"--prefix={prefix}/",
                "-o",
                str(dist / f"{prefix}.tar.gz"),
                "HEAD",
            ],
            root=root,
        )
        run(
            [
                "git",
                "archive",
                "--format=zip",
                f"--prefix={prefix}/",
                "-o",
                str(dist / f"{prefix}.zip"),
                "HEAD",
            ],
            root=root,
        )
    else:
        run(
            [
                "tar",
                "--exclude",
                str(dist),
                "--exclude",
                str(root / "build"),
                "-C",
                str(root),
                "-czf",
                str(dist / f"{prefix}.tar.gz"),
                ".",
            ],
            root=root,
        )
    docs = root / "docs" / "api" / "html"
    if docs.is_dir():
        run(
            ["tar", "-C", str(docs), "-czf", str(dist / f"{prefix}-docs.tar.gz"), "."],
            root=root,
        )
    candidate = root / "dist" / "release-candidate"
    if candidate.is_dir():
        run(
            [
                "tar",
                "-C",
                str(root / "dist"),
                "-czf",
                str(dist / f"{prefix}-verification.tar.gz"),
                "release-candidate",
            ],
            root=root,
        )
    run(
        ["uv", "run", "--script", str(root / "scripts" / "release_notes.py"), tag],
        root=root,
    )
    archives = sorted(
        path
        for path in dist.iterdir()
        if path.is_file()
        and path.name.startswith(prefix)
        and not path.name.endswith(".sha256")
    )
    (dist / f"{prefix}.sha256").write_text(
        "\n".join(
            f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}"
            for path in archives
        )
        + "\n",
        encoding="utf-8",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
