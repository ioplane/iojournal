#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Run the iojournal benchmark profiler review workflows."""

from __future__ import annotations

import argparse
import os
import shlex
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path


BENCHMARKS = {
    "bench_hot_path": {
        "disabled_level",
        "enabled_console",
        "medium_message",
        "medium_message_with_metadata",
        "contention_mpsc",
    },
    "bench_file_sink": {"append_file", "append_ndjson"},
    "bench_syslog_udp": {"udp_loopback"},
}


def root_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def fail(message: str) -> None:
    print(message, file=sys.stderr)
    raise SystemExit(2)


def positive(value: str) -> str:
    if not value.isdigit() or int(value) < 1:
        fail(f"iterations must be a positive integer: {value}")
    return value


def require_command(name: str) -> None:
    if shutil.which(name) is None:
        fail(f"missing tool: {name}")


def require_perf_lane() -> None:
    if os.environ.get("IOJOURNAL_PODMAN_PERF_LANE", "0") != "1":
        fail("this mode must run through scripts/podman_perf_lane.py")


def require_benchmark(path: Path) -> None:
    if not path.is_file() or not path.stat().st_mode & 0o111:
        fail(f"benchmark binary not found: {path}")


def run(command: list[str], root: Path, *, quiet: bool = False) -> None:
    subprocess.run(
        command,
        cwd=root,
        check=True,
        stdout=subprocess.DEVNULL if quiet else None,
    )


def ensure_perf_build(root: Path, preset: str) -> Path:
    run(["cmake", "--preset", preset, "--fresh"], root, quiet=True)
    run(
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
        root,
        quiet=True,
    )
    return root / "build" / preset


def normalize_scenario(benchmark: str, scenario: str) -> str:
    if benchmark not in BENCHMARKS:
        fail(f"unknown bench target: {benchmark}")
    if benchmark == "bench_hot_path":
        selected = scenario or "enabled_console"
    elif benchmark == "bench_file_sink":
        selected = scenario or "append_ndjson"
        if selected == "append_file":
            selected = "append_ndjson"
    else:
        selected = scenario or "udp_loopback"
    if selected not in BENCHMARKS[benchmark]:
        fail(f"unknown scenario for {benchmark}: {scenario}")
    return selected


def bench_args(benchmark: str, iterations: str, scenario: str) -> list[str]:
    arguments = [iterations, "--tsv"]
    if benchmark == "bench_hot_path":
        arguments.extend(("--scenario", scenario))
    return arguments


def benchmark_command(
    build_dir: Path, benchmark: str, iterations: str, scenario: str
) -> list[str]:
    return [
        str(build_dir / "bench" / benchmark),
        *bench_args(benchmark, iterations, scenario),
    ]


def run_callgrind(
    root: Path,
    build_dir: Path,
    results_dir: Path,
    benchmark: str,
    iterations: str,
    scenario: str,
) -> None:
    output = results_dir / f"callgrind-{benchmark}-{scenario}.out"
    command = benchmark_command(build_dir, benchmark, iterations, scenario)
    run(
        [
            "valgrind",
            "--tool=callgrind",
            f"--callgrind-out-file={output}",
            *command,
        ],
        root,
        quiet=True,
    )
    annotate = shutil.which("callgrind_annotate")
    if annotate:
        summary = results_dir / f"callgrind-{benchmark}-{scenario}.summary.txt"
        with summary.open("w", encoding="utf-8") as stream:
            subprocess.run(
                [annotate, "--inclusive=yes", str(output)],
                cwd=root,
                check=True,
                stdout=stream,
            )


def write_auto_summary(
    results_dir: Path,
    run_id: str,
    preset: str,
    iterations: str,
    uftrace_preset: str,
) -> None:
    artifacts = sorted(
        path.name for path in results_dir.iterdir() if path.name != "summary.md"
    )
    lines = [
        "# Profiling Summary",
        "",
        "| Field | Value |",
        "| --- | --- |",
        f"| run_id | `{run_id}` |",
        f"| preset | `{preset}` |",
        "| default_tool | `callgrind` |",
        "| benchmark_targets | `bench_hot_path`, `bench_file_sink` |",
        "| shared_scenarios | `enabled_console`, `medium_message_with_metadata`, `contention_mpsc`, `append_ndjson` |",
        f"| iterations | `{iterations}` |",
        f"| podman_perf_lane | `{os.environ.get('IOJOURNAL_PODMAN_PERF_LANE', '0')}` |",
        f"| uftrace | `{'present' if shutil.which('uftrace') else 'missing'}` |",
        f"| hyperfine | `{'present' if shutil.which('hyperfine') else 'missing'}` |",
        f"| callgrind_annotate | `{'present' if shutil.which('callgrind_annotate') else 'missing'}` |",
        f"| uftrace_preset | `{uftrace_preset}` |",
        "",
        "## Artifacts",
        "",
        *[f"- `{artifact}`" for artifact in artifacts],
        "",
    ]
    (results_dir / "summary.md").write_text("\n".join(lines), encoding="utf-8")


def run_auto(
    root: Path,
    build_dir: Path,
    results_dir: Path,
    run_id: str,
    preset: str,
    iterations: str,
    uftrace_preset: str,
) -> int:
    results_dir.mkdir(parents=True, exist_ok=True)
    require_command("valgrind")
    ensure_perf_build(root, preset)
    for benchmark, scenario in (
        ("bench_hot_path", "enabled_console"),
        ("bench_hot_path", "medium_message_with_metadata"),
        ("bench_hot_path", "contention_mpsc"),
        ("bench_file_sink", "append_ndjson"),
    ):
        run_callgrind(root, build_dir, results_dir, benchmark, iterations, scenario)

    if shutil.which("hyperfine"):
        for benchmark, scenario in (
            ("bench_hot_path", "medium_message_with_metadata"),
            ("bench_hot_path", "contention_mpsc"),
            ("bench_file_sink", "append_ndjson"),
        ):
            command = shlex.join(
                benchmark_command(build_dir, benchmark, iterations, scenario)
            )
            run(
                [
                    "hyperfine",
                    "--warmup",
                    "3",
                    "--export-markdown",
                    str(results_dir / f"hyperfine-{benchmark}-{scenario}.md"),
                    command,
                ],
                root,
            )
    write_auto_summary(results_dir, run_id, preset, iterations, uftrace_preset)
    print(results_dir)
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "tool", choices=("auto", "uftrace", "callgrind", "gdb", "hyperfine")
    )
    parser.add_argument("arg2", nargs="?")
    parser.add_argument("arg3", nargs="?")
    parser.add_argument("arg4", nargs="?")
    args = parser.parse_args()

    root = root_dir()
    preset = os.environ.get("PRESET", "clang-perf")
    run_id = os.environ.get("RUN_ID", datetime.now().strftime("%Y%m%d-%H%M%S"))
    results_root = Path(
        os.environ.get("RESULTS_DIR", str(root / "docs" / "tmp" / "profiling"))
    )
    results_dir = results_root / run_id
    results_dir.mkdir(parents=True, exist_ok=True)
    uftrace_preset = os.environ.get("UFTRACE_PRESET", "clang-uftrace")
    uftrace_build = Path(
        os.environ.get("UFTRACE_BUILD_DIR", str(root / "build" / uftrace_preset))
    )

    if args.tool == "auto":
        iterations = positive(args.arg2 or "5000")
        build_dir = root / "build" / preset
        return run_auto(
            root, build_dir, results_dir, run_id, preset, iterations, uftrace_preset
        )

    benchmark = args.arg2 or "bench_hot_path"
    iterations = positive(args.arg3 or "5000")
    scenario = normalize_scenario(benchmark, args.arg4 or "")
    command_args = bench_args(benchmark, iterations, scenario)

    if args.tool == "uftrace":
        require_perf_lane()
        require_command("uftrace")
        run(
            ["uv", "run", "--script", str(root / "scripts" / "build_uftrace_bench.py")],
            root,
            quiet=True,
        )
        run(
            [
                "uftrace",
                "record",
                "-d",
                str(results_dir / f"uftrace-{benchmark}-{scenario}"),
                "--",
                str(uftrace_build / "bench" / benchmark),
                *command_args,
            ],
            root,
        )
        return 0

    build_dir = ensure_perf_build(root, preset)
    benchmark_path = build_dir / "bench" / benchmark
    require_benchmark(benchmark_path)
    if args.tool == "callgrind":
        require_command("valgrind")
        run_callgrind(root, build_dir, results_dir, benchmark, iterations, scenario)
        print(results_dir)
        return 0
    if args.tool == "gdb":
        require_perf_lane()
        require_command("gdb")
        run(["gdb", "--args", str(benchmark_path), *command_args], root)
        return 0
    require_command("hyperfine")
    command = shlex.join([str(benchmark_path), *command_args])
    run(
        [
            "hyperfine",
            "--warmup",
            "3",
            "--export-markdown",
            str(results_dir / f"hyperfine-{benchmark}-{scenario}.md"),
            command,
        ],
        root,
    )
    print(results_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
