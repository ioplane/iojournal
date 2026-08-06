#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Build and run the normalized Tier-1 competitor benchmark suite."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
from datetime import datetime
from pathlib import Path


def root_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def positive_env(name: str, default: str) -> str:
    value = os.environ.get(name, default)
    if not value.isdigit() or int(value) < 1:
        raise SystemExit(f"{name} must be a positive integer")
    return value


def run_capture(
    command: list[str], *, cwd: Path, environment: dict[str, str] | None = None
) -> str:
    result = subprocess.run(
        command,
        cwd=cwd,
        check=True,
        env=environment,
        text=True,
        stdout=subprocess.PIPE,
    )
    print(result.stdout, end="")
    return result.stdout


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    root = root_dir()
    tier1_root = Path(os.environ.get("TIER1_ROOT", str(root / "build" / "tier1")))
    source_root = Path(os.environ.get("SRC_ROOT", str(tier1_root / "src")))
    build_root = Path(os.environ.get("BUILD_ROOT", str(tier1_root / "build")))
    binary_root = Path(os.environ.get("BIN_ROOT", str(tier1_root / "bin")))
    library_root = Path(os.environ.get("LIB_ROOT", str(tier1_root / "lib")))
    results_root = Path(
        os.environ.get("RESULTS_ROOT", str(root / "docs" / "tmp" / "benchmarks"))
    )
    run_id = os.environ.get("RUN_ID", datetime.now().strftime("%Y%m%d-%H%M%S"))
    run_dir = results_root / run_id
    iterations = positive_env("HOT_ITERATIONS", "50000")
    run_dir.mkdir(parents=True, exist_ok=True)
    for path in (build_root, binary_root, library_root):
        path.mkdir(parents=True, exist_ok=True)

    repositories = ("zlog", "stumpless", "tinylog")
    missing = [name for name in repositories if not (source_root / name).is_dir()]
    if missing:
        raise SystemExit(
            f"Tier 1 source trees are missing under {source_root}: {', '.join(missing)}; "
            "run scripts/tier1.py fetch first"
        )

    jobs = str(os.cpu_count() or 1)
    zlog_build = build_root / "zlog-upstream"
    stumpless_build = build_root / "stumpless-upstream"
    tinylog_build = build_root / "tinylog-upstream"
    run(
        [
            "cmake",
            "-S",
            str(source_root / "zlog"),
            "-B",
            str(zlog_build),
            "-DCMAKE_BUILD_TYPE=Release",
        ],
        root,
    )
    run(["cmake", "--build", str(zlog_build), f"-j{jobs}"], root)
    run(
        [
            "cmake",
            "-S",
            str(source_root / "stumpless"),
            "-B",
            str(stumpless_build),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DBUILD_TESTING=ON",
            "-DBUILD_BENCHMARKING=OFF",
            "-DINSTALL_HTML=OFF",
            "-DINSTALL_MANPAGES=OFF",
            "-DINSTALL_EXAMPLES=OFF",
        ],
        root,
    )
    run(
        [
            "cmake",
            "--build",
            str(stumpless_build),
            f"-j{jobs}",
            "--target",
            "stumpless",
        ],
        root,
    )
    run(
        [
            "cmake",
            "-S",
            str(source_root / "tinylog"),
            "-B",
            str(tinylog_build),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        ],
        root,
    )
    run(["cmake", "--build", str(tinylog_build), f"-j{jobs}"], root)

    libraries = (
        (zlog_build / "lib" / "libzlog.so", library_root / "libzlog.so"),
        (stumpless_build / "libstumpless.so", library_root / "libstumpless.so"),
        (tinylog_build / "libtlog.so", library_root / "libtlog.so"),
    )
    for source, destination in libraries:
        shutil.copy2(source, destination)

    common_flags = [
        "-std=c23",
        "-D_POSIX_C_SOURCE=200809L",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
    ]
    compile_commands = (
        (
            "bench_zlog",
            [
                *common_flags,
                f"-I{root / 'bench' / 'tier1'}",
                f"-I{source_root / 'zlog' / 'src'}",
                str(root / "bench" / "tier1" / "bench_zlog.c"),
                f"-L{library_root}",
                f"-Wl,-rpath,{library_root}",
                "-lzlog",
                "-lpthread",
                "-o",
                str(binary_root / "bench_zlog"),
            ],
        ),
        (
            "bench_stumpless",
            [
                *common_flags,
                f"-I{root / 'bench' / 'tier1'}",
                f"-I{source_root / 'stumpless' / 'include'}",
                f"-I{stumpless_build / 'include'}",
                str(root / "bench" / "tier1" / "bench_stumpless.c"),
                f"-L{library_root}",
                f"-Wl,-rpath,{library_root}",
                "-lstumpless",
                "-lpthread",
                "-o",
                str(binary_root / "bench_stumpless"),
            ],
        ),
        (
            "bench_tinylog",
            [
                *common_flags,
                f"-I{root / 'bench' / 'tier1'}",
                f"-I{source_root / 'tinylog'}",
                str(root / "bench" / "tier1" / "bench_tinylog.c"),
                f"-L{library_root}",
                f"-Wl,-rpath,{library_root}",
                "-ltlog",
                "-lpthread",
                "-o",
                str(binary_root / "bench_tinylog"),
            ],
        ),
    )
    for _, command in compile_commands:
        run(["cc", *command], root)

    environment = os.environ.copy()
    old_library_path = environment.get("LD_LIBRARY_PATH")
    search_path = ":".join(
        str(path)
        for path in (
            library_root,
            zlog_build / "lib",
            stumpless_build,
            tinylog_build,
        )
    )
    environment["LD_LIBRARY_PATH"] = (
        f"{search_path}:{old_library_path}" if old_library_path else search_path
    )
    for name in ("bench_zlog", "bench_stumpless", "bench_tinylog"):
        output = run_capture(
            [str(binary_root / name), iterations, "--tsv"],
            cwd=root,
            environment=environment,
        )
        (run_dir / f"{name}.tsv").write_text(output, encoding="utf-8")
        if "format\ttsv\tv1\n" not in output:
            raise SystemExit(f"missing TSV format marker: {name}")

    (run_dir / "tier1-manifest.md").write_text(
        "# Tier 1 Benchmark Manifest\n\n"
        "| Field | Value |\n| --- | --- |\n"
        f"| run_id | `{run_id}` |\n"
        f"| shared_iterations | `{iterations}` |\n"
        "| libraries | `zlog`, `stumpless`, `tinylog` |\n"
        "| benchmark_scope | shared Tier 1 scenarios plus normalized file append |\n\n"
        "## Artifacts\n\n"
        "- `bench_zlog.tsv`\n- `bench_stumpless.tsv`\n- `bench_tinylog.tsv`\n",
        encoding="utf-8",
    )
    print(run_dir)
    return 0


def run(command: list[str], root: Path) -> None:
    subprocess.run(command, cwd=root, check=True)


if __name__ == "__main__":
    raise SystemExit(main())
