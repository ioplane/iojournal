#!/usr/bin/env bash
# shellcheck shell=bash
# Builds the benchmark targets with the uftrace-instrumented preset.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

readonly PRESET="${PRESET:-clang-uftrace}"

cmake --preset "${PRESET}" --fresh
cmake --build --preset "${PRESET}" --target bench_hot_path bench_file_sink bench_syslog_udp
