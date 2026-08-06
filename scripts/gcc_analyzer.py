#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Run the iojournal GCC analyzer lane and preserve warning evidence."""

from __future__ import annotations

import os
import re
import shutil
import subprocess
from pathlib import Path


KNOWN_WARNING_RE = re.compile(
    r"analyzer-too-complex|analyzer-symbol-too-complex|analyzer-malloc-leak|"
    r"analyzer-fd-leak|format-truncation|format-overflow"
)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    preset = os.environ.get("PRESET", "gcc-analyzer")
    build_dir = Path(os.environ.get("BUILD_DIR", root / "build" / preset))
    report_dir = build_dir / "analyzer"
    report_dir.mkdir(parents=True, exist_ok=True)

    if (
        not (root / "CMakeLists.txt").is_file()
        or not (root / "CMakePresets.json").is_file()
    ):
        (report_dir / "summary.txt").write_text(
            "SKIP: GCC analyzer unavailable until the CMake project is initialized\n",
            encoding="utf-8",
        )
        print((report_dir / "summary.txt").read_text(encoding="utf-8"), end="")
        return 0
    if shutil.which("gcc") is None:
        (report_dir / "summary.txt").write_text(
            "SKIP: gcc is not available in the current environment\n", encoding="utf-8"
        )
        print((report_dir / "summary.txt").read_text(encoding="utf-8"), end="")
        return 0

    build_log = report_dir / "build.log"
    tests_log = report_dir / "tests.log"
    configure = subprocess.run(["cmake", "--preset", preset], cwd=root, check=True)
    del configure
    with build_log.open("w", encoding="utf-8") as output:
        subprocess.run(
            ["cmake", "--build", "--preset", preset],
            cwd=root,
            stdout=output,
            stderr=subprocess.STDOUT,
            check=True,
        )
    with tests_log.open("w", encoding="utf-8") as output:
        subprocess.run(
            ["ctest", "--preset", preset, "--output-on-failure"],
            cwd=root,
            stdout=output,
            stderr=subprocess.STDOUT,
            check=True,
        )

    warnings = [
        line
        for line in build_log.read_text(encoding="utf-8").splitlines()
        if "warning:" in line
    ]
    known = [line for line in warnings if KNOWN_WARNING_RE.search(line)]
    unexpected = [line for line in warnings if not KNOWN_WARNING_RE.search(line)]
    summary = (
        "PASS: gcc analyzer lane completed\n"
        f"PRESET={preset}\nBUILD_DIR={build_dir}\n"
        f"TOTAL_WARNINGS={len(warnings)}\nKNOWN_WARNINGS={len(known)}\n"
        f"OTHER_WARNINGS={len(unexpected)}\n"
    )
    (report_dir / "summary.txt").write_text(summary, encoding="utf-8")
    (report_dir / "known-warnings.txt").write_text(
        "\n".join(known) + "\n", encoding="utf-8"
    )
    if unexpected:
        (report_dir / "unexpected-warnings.txt").write_text(
            "\n".join(unexpected) + "\n", encoding="utf-8"
        )
        print(summary, end="")
        print("\n".join(unexpected))
        return 1
    print(summary, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
