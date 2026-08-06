#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Run ptrace-sensitive iojournal work inside the official Podman perf lane."""

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

IMAGE = "localhost/iojournal-dev:latest"
WORKSPACE_CONTAINER = "/workspace"


def usage() -> None:
    print(
        """usage: uv run --script scripts/podman_perf_lane.py [command...]

Run a command inside the official Podman performance lane for iojournal.

The lane is reserved for io_uring, uftrace, callgrind, hyperfine, gdb, and
other ptrace-sensitive diagnostics. The default command is `bash`.

Examples:
  uv run --script scripts/podman_perf_lane.py bash
  uv run --script scripts/podman_perf_lane.py uv run --script scripts/profiler_review.py auto 3000
"""
    )


def main() -> int:
    if len(sys.argv) > 1 and sys.argv[1] in {"--help", "-h"}:
        usage()
        return 0
    root = Path(__file__).resolve().parents[1]
    podman = os.environ.get("PODMAN_BIN", "podman")
    command = sys.argv[1:] or ["bash"]
    podman_args = [
        podman,
        "run",
        "--rm",
        "--security-opt",
        "seccomp=unconfined",
        "--cap-add",
        "SYS_PTRACE",
        "--env",
        "IOJOURNAL_PODMAN_PERF_LANE=1",
        "-v",
        f"{root}:{WORKSPACE_CONTAINER}:Z",
        "-w",
        WORKSPACE_CONTAINER,
        IMAGE,
        *command,
    ]
    if sys.stdin.isatty() and sys.stdout.isatty():
        podman_args[2:2] = ["-it"]
    return subprocess.run(podman_args, cwd=root, check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
