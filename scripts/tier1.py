#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Fetch and build the iojournal Tier-1 logging competitors."""

from __future__ import annotations

import argparse
import os
import subprocess
from pathlib import Path


REPOSITORIES = {
    "zlog": "https://github.com/HardySimpson/zlog.git",
    "stumpless": "https://github.com/goatshriek/stumpless.git",
    "tinylog": "https://github.com/pymumu/tinylog.git",
}


def root_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def source_root() -> Path:
    root = root_dir()
    return Path(
        os.environ.get("SRC_ROOT", str(root / "docs" / "tmp" / "competitors" / "src"))
    )


def run(command: list[str], *, cwd: Path | None = None) -> None:
    subprocess.run(command, cwd=cwd or root_dir(), check=True)


def fetch() -> int:
    sources = source_root()
    sources.mkdir(parents=True, exist_ok=True)
    for name, url in REPOSITORIES.items():
        repository = sources / name
        if not (repository / ".git").is_dir():
            run(["git", "clone", "--depth", "1", url, str(repository)])
        else:
            run(["git", "pull", "--ff-only"], cwd=repository)
    print(sources)
    return 0


def build() -> int:
    fetch()
    root = root_dir()
    sources = source_root()
    build_root = Path(
        os.environ.get("BUILD_ROOT", str(root / "build" / "tier1" / "build"))
    )
    install_root = Path(
        os.environ.get("INSTALL_ROOT", str(root / "build" / "tier1" / "install"))
    )
    build_root.mkdir(parents=True, exist_ok=True)
    install_root.mkdir(parents=True, exist_ok=True)

    zlog_build = build_root / "zlog"
    zlog_install = install_root / "zlog"
    run(
        [
            "cmake",
            "--fresh",
            "-S",
            str(sources / "zlog"),
            "-B",
            str(zlog_build),
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_INSTALL_PREFIX={zlog_install}",
        ]
    )
    run(
        [
            "cmake",
            "--build",
            str(zlog_build),
            "--target",
            "zlog",
            "zlog_s",
            "zlog-chk-conf",
        ]
    )
    run(["cmake", "--install", str(zlog_build)])

    stumpless_build = build_root / "stumpless"
    stumpless_install = install_root / "stumpless"
    compatibility = build_root / "stumpless-compat.cmake"
    compatibility.write_text(
        """if(NOT COMMAND add_function_test)
    function(add_function_test)
    endfunction()
endif()
if(NOT COMMAND add_cpp_test)
    function(add_cpp_test)
    endfunction()
endif()
if(NOT COMMAND add_fuzz_test)
    function(add_fuzz_test)
    endfunction()
endif()
if(NOT COMMAND add_performance_test)
    function(add_performance_test)
    endfunction()
endif()
if(NOT COMMAND add_single_file_function_test)
    function(add_single_file_function_test)
    endfunction()
endif()
if(NOT COMMAND add_single_file_performance_test)
    function(add_single_file_performance_test)
    endfunction()
endif()
if(NOT COMMAND add_thread_safety_test)
    function(add_thread_safety_test)
    endfunction()
endif()
""",
        encoding="utf-8",
    )
    run(
        [
            "cmake",
            "--fresh",
            "-S",
            str(sources / "stumpless"),
            "-B",
            str(stumpless_build),
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_INSTALL_PREFIX={stumpless_install}",
            f"-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES={compatibility}",
            "-DBUILD_TESTING=OFF",
            "-DBUILD_BENCHMARKING=OFF",
            "-DBUILD_CPP=OFF",
            "-DBUILD_PYTHON=OFF",
            "-DINSTALL_HTML=OFF",
            "-DINSTALL_MANPAGES=OFF",
            "-DINSTALL_EXAMPLES=OFF",
        ]
    )
    run(["cmake", "--build", str(stumpless_build), "--target", "stumpless"])
    run(["cmake", "--install", str(stumpless_build)])

    tinylog_build = build_root / "tinylog"
    tinylog_install = install_root / "tinylog"
    run(
        [
            "cmake",
            "--fresh",
            "-S",
            str(sources / "tinylog"),
            "-B",
            str(tinylog_build),
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_INSTALL_PREFIX={tinylog_install}",
            "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        ]
    )
    run(["cmake", "--build", str(tinylog_build), "--target", "tlog"])
    run(["cmake", "--install", str(tinylog_build)])

    (install_root / "README.md").write_text(
        "# Tier 1 Competitor Install Roots\n\n"
        "| Library | Install Prefix |\n| --- | --- |\n"
        f"| `zlog` | `{install_root / 'zlog'}` |\n"
        f"| `stumpless` | `{install_root / 'stumpless'}` |\n"
        f"| `tinylog` | `{install_root / 'tinylog'}` |\n",
        encoding="utf-8",
    )
    print(install_root)
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("fetch", "build"))
    args = parser.parse_args()
    return fetch() if args.command == "fetch" else build()


if __name__ == "__main__":
    raise SystemExit(main())
