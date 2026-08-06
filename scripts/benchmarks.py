#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.14,<3.15"
# dependencies = []
# ///
"""Build and execute the iojournal benchmark suite."""

from __future__ import annotations

import argparse
import os
import shutil
import shlex
import subprocess
from datetime import datetime
from pathlib import Path


SCENARIOS = (
    ("disabled_level", "shared", "active"),
    ("enabled_console", "shared", "active"),
    ("append_ndjson", "shared", "active"),
    ("udp_loopback", "capability-specific", "active"),
    ("medium_message", "shared", "active"),
    ("medium_message_with_metadata", "shared", "active"),
    ("contention_mpsc", "shared", "active"),
)


def root_dir() -> Path:
    return Path(__file__).resolve().parents[1]


def positive_env(name: str, default: str) -> str:
    value = os.environ.get(name, default)
    if not value.isdigit() or int(value) < 1:
        raise SystemExit(f"{name} must be a positive integer")
    return value


def run_capture(command: list[str], root: Path) -> str:
    result = subprocess.run(
        command, cwd=root, check=True, text=True, stdout=subprocess.PIPE
    )
    print(result.stdout, end="")
    return result.stdout


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    root = root_dir()
    preset = os.environ.get("PRESET", "clang-debug")
    build_dir = Path(os.environ.get("BUILD_DIR", str(root / "build" / preset)))
    results_root = Path(
        os.environ.get("RESULTS_ROOT", str(root / "docs" / "tmp" / "benchmarks"))
    )
    run_id = os.environ.get("RUN_ID", datetime.now().strftime("%Y%m%d-%H%M%S"))
    run_dir = results_root / run_id
    hot_iterations = positive_env("HOT_ITERATIONS", "50000")
    file_iterations = positive_env("FILE_ITERATIONS", hot_iterations)
    syslog_iterations = positive_env("SYSLOG_ITERATIONS", "5000")
    run_dir.mkdir(parents=True, exist_ok=True)

    subprocess.run(["cmake", "--preset", preset], cwd=root, check=True)
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

    benchmarks = (
        ("bench_hot_path", hot_iterations),
        ("bench_file_sink", file_iterations),
        ("bench_syslog_udp", syslog_iterations),
    )
    for name, iterations in benchmarks:
        output = run_capture(
            [str(build_dir / "bench" / name), iterations, "--tsv"], root
        )
        (run_dir / f"{name}.tsv").write_text(output, encoding="utf-8")

    if shutil.which("hyperfine"):
        commands = [
            shlex.join(
                [
                    str(build_dir / "bench" / "bench_hot_path"),
                    hot_iterations,
                    "--tsv",
                    "--scenario",
                    "disabled_level",
                ]
            ),
            shlex.join(
                [
                    str(build_dir / "bench" / "bench_hot_path"),
                    hot_iterations,
                    "--tsv",
                    "--scenario",
                    "enabled_console",
                ]
            ),
        ]
        subprocess.run(
            [
                "hyperfine",
                "--warmup",
                "2",
                "--export-markdown",
                str(run_dir / "hyperfine.md"),
                *commands,
            ],
            cwd=root,
            check=True,
        )

    scenario_file = run_dir / "scenario-status.tsv"
    scenario_file.write_text(
        "scenario\tclass\tstatus\n"
        + "\n".join("\t".join(row) for row in SCENARIOS)
        + "\n",
        encoding="utf-8",
    )
    hyperfine_status = "present" if shutil.which("hyperfine") else "missing"
    artifacts = [
        "bench_hot_path.tsv",
        "bench_file_sink.tsv",
        "bench_syslog_udp.tsv",
        "scenario-status.tsv",
    ]
    if (run_dir / "hyperfine.md").is_file():
        artifacts.append("hyperfine.md")
    manifest = [
        "# Benchmark Run Manifest",
        "",
        "| Field | Value |",
        "| --- | --- |",
        f"| run_id | `{run_id}` |",
        f"| preset | `{preset}` |",
        f"| build_dir | `{build_dir}` |",
        f"| hot_iterations | `{hot_iterations}` |",
        f"| file_iterations | `{file_iterations}` |",
        f"| syslog_iterations | `{syslog_iterations}` |",
        f"| hyperfine | `{hyperfine_status}` |",
        "",
        "## Artifacts",
        "",
        *[f"- `{artifact}`" for artifact in artifacts],
        "",
        "## Scenario Status",
        "",
        "| Scenario | Class | Status |",
        "| --- | --- | --- |",
        *[f"| `{name}` | `{kind}` | `{status}` |" for name, kind, status in SCENARIOS],
        "",
    ]
    (run_dir / "manifest.md").write_text("\n".join(manifest), encoding="utf-8")

    for name, _ in benchmarks:
        artifact = (run_dir / f"{name}.tsv").read_text(encoding="utf-8")
        if "format\ttsv\tv1\n" not in artifact:
            raise SystemExit(f"missing TSV format marker: {name}")
        if (
            "columns\tbenchmark\tscenario\titerations\telapsed_ns\tns_per_op\n"
            not in artifact
        ):
            raise SystemExit(f"missing TSV columns marker: {name}")
    print(run_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
