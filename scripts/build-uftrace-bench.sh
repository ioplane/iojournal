#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PRESET="${PRESET:-clang-uftrace}"

cmake --preset "${PRESET}" --fresh
cmake --build --preset "${PRESET}" --target bench_hot_path bench_file_sink bench_syslog_udp
