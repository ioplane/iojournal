#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Build iojournal benchmarks with the uftrace-instrumented CMake preset."""

from __future__ import annotations

import os
import subprocess
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    preset = os.environ.get("PRESET", "clang-uftrace")
    subprocess.run(["cmake", "--preset", preset, "--fresh"], cwd=root, check=True)
    subprocess.run(
        [
            "cmake",
            "--build",
            "--preset",
            preset,
            "--target",
            "bench_hot_path",
            "bench_file_sink",
            "bench_syslog_udp",
        ],
        cwd=root,
        check=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
