#!/usr/bin/env bash
# shellcheck shell=bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
readonly ROOT_DIR
readonly PRESET="${PRESET:-clang-perf}"
readonly BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/${PRESET}}"
RUN_ID="${RUN_ID:-$(date +%Y%m%d-%H%M%S)}"
readonly RUN_ID
TOOL="${1:-auto}"
BENCH_NAME="bench_hot_path"
ITERATIONS="5000"
SCENARIO=""
readonly RESULTS_ROOT="${RESULTS_DIR:-${ROOT_DIR}/docs/tmp/profiling}"
readonly RESULTS_DIR="${RESULTS_ROOT}/${RUN_ID}"
readonly SUMMARY_FILE="${RESULTS_DIR}/summary.md"
readonly UFTRACE_PRESET="${UFTRACE_PRESET:-clang-uftrace}"
readonly UFTRACE_BUILD_DIR="${UFTRACE_BUILD_DIR:-${ROOT_DIR}/build/${UFTRACE_PRESET}}"
readonly PODMAN_PERF_LANE="${IOJOURNAL_PODMAN_PERF_LANE:-0}"

usage() {
    cat <<'EOF'
usage:
  scripts/run-profiler-review.sh auto [iterations]
  scripts/run-profiler-review.sh <tool> [bench_name] [iterations] [scenario]

For host-launched io_uring or ptrace-sensitive runs, invoke this script through:
  uv run --script scripts/podman_perf_lane.py bash scripts/run-profiler-review.sh ...

Notes:
  auto ignores bench_name and scenario, and always runs the canonical shared suite.

tool:
  auto
  uftrace
  callgrind
  gdb
  hyperfine

bench_name:
  bench_hot_path
  bench_file_sink
  bench_syslog_udp

scenario:
  bench_hot_path: disabled_level|enabled_console|medium_message|medium_message_with_metadata|contention_mpsc
  bench_file_sink: append_ndjson
  bench_syslog_udp: udp_loopback
EOF
}

need_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        printf "missing tool: %s\n" "$1" >&2
        exit 2
    fi
}

validate_iterations() {
    if [[ ! "$1" =~ ^[0-9]+$ ]] || (( $1 == 0 )); then
        printf "iterations must be a positive integer: %s\n" "$1" >&2
        exit 2
    fi
}

require_perf_lane() {
    if [[ "${PODMAN_PERF_LANE}" != "1" ]]; then
        printf "this mode must run through scripts/podman_perf_lane.py\n" >&2
        exit 2
    fi
}

ensure_perf_build() {
    cmake --preset "${PRESET}" --fresh >/dev/null
    cmake --build --preset "${PRESET}" \
        --target bench_hot_path bench_file_sink bench_syslog_udp >/dev/null
}

ensure_uftrace_build() {
    bash "${ROOT_DIR}/scripts/build-uftrace-bench.sh" >/dev/null
}

run_callgrind() {
    local bench_name="$1"
    local iterations="$2"
    local scenario_name="$3"
    local bench_bin="${BUILD_DIR}/bench/${bench_name}"
    local -a bench_argv
    local output_file
    local summary_file

    mapfile -t bench_argv < <(bench_args "${bench_name}" "${iterations}" "${scenario_name}")
    output_file="${RESULTS_DIR}/callgrind-${bench_name}-${scenario_name}.out"
    summary_file="${RESULTS_DIR}/callgrind-${bench_name}-${scenario_name}.summary.txt"

    valgrind --tool=callgrind \
        --callgrind-out-file="${output_file}" \
        "${bench_bin}" "${bench_argv[@]}" >/dev/null

    if command -v callgrind_annotate >/dev/null 2>&1; then
        callgrind_annotate --inclusive=yes "${output_file}" >"${summary_file}"
    fi
}

normalize_scenario() {
    case "$1" in
    bench_hot_path)
        case "${2:-enabled_console}" in
        disabled_level|enabled_console|medium_message|medium_message_with_metadata|contention_mpsc)
            printf '%s\n' "${2:-enabled_console}"
            ;;
        *)
            printf "unknown scenario for bench_hot_path: %s\n" "${2:-}" >&2
            exit 2
            ;;
        esac
        ;;
    bench_file_sink)
        case "${2:-append_ndjson}" in
        append_file|append_ndjson)
            printf 'append_ndjson\n'
            ;;
        *)
            printf "unknown scenario for bench_file_sink: %s\n" "${2:-}" >&2
            exit 2
            ;;
        esac
        ;;
    bench_syslog_udp)
        case "${2:-udp_loopback}" in
        udp_loopback)
            printf 'udp_loopback\n'
            ;;
        *)
            printf "unknown scenario for bench_syslog_udp: %s\n" "${2:-}" >&2
            exit 2
            ;;
        esac
        ;;
    *)
        printf "unknown bench target: %s\n" "$1" >&2
        exit 2
        ;;
    esac
}

bench_args() {
    local bench_name="$1"
    local iterations="$2"
    local scenario_name

    scenario_name="$(normalize_scenario "${bench_name}" "${3:-}")"

    case "${bench_name}" in
    bench_hot_path)
        printf '%s\n' "${iterations}" "--tsv" "--scenario" "${scenario_name}"
        ;;
    bench_file_sink)
        printf '%s\n' "${iterations}" "--tsv"
        ;;
    bench_syslog_udp)
        printf '%s\n' "${iterations}" "--tsv"
        ;;
    *)
        printf "unknown bench target: %s\n" "${bench_name}" >&2
        exit 2
        ;;
    esac
}

command_string() {
    local rendered=""
    local arg

    for arg in "$@"; do
        printf -v rendered '%s%q ' "${rendered}" "${arg}"
    done

    printf '%s\n' "${rendered% }"
}

if [[ -z "${TOOL}" ]]; then
    usage
    exit 2
fi

if [[ "${TOOL}" == "auto" ]]; then
    ITERATIONS="${2:-5000}"
else
    BENCH_NAME="${2:-bench_hot_path}"
    ITERATIONS="${3:-5000}"
    SCENARIO="${4:-}"
fi

validate_iterations "${ITERATIONS}"
mkdir -p "${RESULTS_DIR}"

case "${TOOL}" in
auto)
    local_bench_hot_path_medium_message_with_metadata=(
        "${BUILD_DIR}/bench/bench_hot_path" "${ITERATIONS}" --tsv --scenario medium_message_with_metadata
    )
    local_bench_hot_path_contention_mpsc=(
        "${BUILD_DIR}/bench/bench_hot_path" "${ITERATIONS}" --tsv --scenario contention_mpsc
    )
    local_bench_file_sink_append_ndjson=(
        "${BUILD_DIR}/bench/bench_file_sink" "${ITERATIONS}" --tsv
    )
    need_cmd valgrind
    ensure_perf_build
    run_callgrind bench_hot_path "${ITERATIONS}" enabled_console
    run_callgrind bench_hot_path "${ITERATIONS}" medium_message_with_metadata
    run_callgrind bench_hot_path "${ITERATIONS}" contention_mpsc
    run_callgrind bench_file_sink "${ITERATIONS}" append_ndjson
    if command -v hyperfine >/dev/null 2>&1; then
        hyperfine \
            --warmup 3 \
            --export-markdown "${RESULTS_DIR}/hyperfine-bench_hot_path-medium_message_with_metadata.md" \
            "$(command_string "${local_bench_hot_path_medium_message_with_metadata[@]}")"
        hyperfine \
            --warmup 3 \
            --export-markdown "${RESULTS_DIR}/hyperfine-bench_hot_path-contention_mpsc.md" \
            "$(command_string "${local_bench_hot_path_contention_mpsc[@]}")"
        hyperfine \
            --warmup 3 \
            --export-markdown "${RESULTS_DIR}/hyperfine-bench_file_sink-append_ndjson.md" \
            "$(command_string "${local_bench_file_sink_append_ndjson[@]}")"
    fi
    # shellcheck disable=SC2016
    {
        printf '# Profiling Summary\n\n'
        printf '| Field | Value |\n'
        printf '| --- | --- |\n'
        printf '| run_id | `%s` |\n' "${RUN_ID}"
        printf '| preset | `%s` |\n' "${PRESET}"
        printf '| default_tool | `callgrind` |\n'
        printf '| benchmark_targets | `bench_hot_path`, `bench_file_sink` |\n'
        printf '| shared_scenarios | `enabled_console`, `medium_message_with_metadata`, `contention_mpsc`, `append_ndjson` |\n'
        printf '| iterations | `%s` |\n' "${ITERATIONS}"
        printf '| podman_perf_lane | `%s` |\n' "${PODMAN_PERF_LANE}"
        printf '| uftrace | `%s` |\n' "$(command -v uftrace >/dev/null 2>&1 && printf 'present' || printf 'missing')"
        printf '| hyperfine | `%s` |\n' "$(command -v hyperfine >/dev/null 2>&1 && printf 'present' || printf 'missing')"
        printf '| callgrind_annotate | `%s` |\n' "$(command -v callgrind_annotate >/dev/null 2>&1 && printf 'present' || printf 'missing')"
        printf '| uftrace_preset | `%s` |\n' "${UFTRACE_PRESET}"
        printf '\n## Artifacts\n\n'
        printf -- '- `callgrind-bench_hot_path-enabled_console.out`\n'
        printf -- '- `callgrind-bench_hot_path-medium_message_with_metadata.out`\n'
        printf -- '- `callgrind-bench_hot_path-contention_mpsc.out`\n'
        printf -- '- `callgrind-bench_file_sink-append_ndjson.out`\n'
        if [[ -f "${RESULTS_DIR}/callgrind-bench_hot_path-enabled_console.summary.txt" ]]; then
            printf -- '- `callgrind-bench_hot_path-enabled_console.summary.txt`\n'
        fi
        if [[ -f "${RESULTS_DIR}/callgrind-bench_hot_path-medium_message_with_metadata.summary.txt" ]]; then
            printf -- '- `callgrind-bench_hot_path-medium_message_with_metadata.summary.txt`\n'
        fi
        if [[ -f "${RESULTS_DIR}/callgrind-bench_hot_path-contention_mpsc.summary.txt" ]]; then
            printf -- '- `callgrind-bench_hot_path-contention_mpsc.summary.txt`\n'
        fi
        if [[ -f "${RESULTS_DIR}/callgrind-bench_file_sink-append_ndjson.summary.txt" ]]; then
            printf -- '- `callgrind-bench_file_sink-append_ndjson.summary.txt`\n'
        fi
        if [[ -f "${RESULTS_DIR}/hyperfine-bench_hot_path-medium_message_with_metadata.md" ]]; then
            printf -- '- `hyperfine-bench_hot_path-medium_message_with_metadata.md`\n'
        fi
        if [[ -f "${RESULTS_DIR}/hyperfine-bench_hot_path-contention_mpsc.md" ]]; then
            printf -- '- `hyperfine-bench_hot_path-contention_mpsc.md`\n'
        fi
        if [[ -f "${RESULTS_DIR}/hyperfine-bench_file_sink-append_ndjson.md" ]]; then
            printf -- '- `hyperfine-bench_file_sink-append_ndjson.md`\n'
        fi
    } >"${SUMMARY_FILE}"
    printf '%s\n' "${RESULTS_DIR}"
    ;;
uftrace)
    require_perf_lane
    need_cmd uftrace
    ensure_uftrace_build
    SCENARIO_NAME="$(normalize_scenario "${BENCH_NAME}" "${SCENARIO}")"
    mapfile -t BENCH_ARGS < <(bench_args "${BENCH_NAME}" "${ITERATIONS}" "${SCENARIO_NAME}")
    exec uftrace record -d "${RESULTS_DIR}/uftrace-${BENCH_NAME}-${SCENARIO_NAME}" -- \
        "${UFTRACE_BUILD_DIR}/bench/${BENCH_NAME}" "${BENCH_ARGS[@]}"
    ;;
callgrind)
    need_cmd valgrind
    ensure_perf_build
    SCENARIO_NAME="$(normalize_scenario "${BENCH_NAME}" "${SCENARIO}")"
    mapfile -t BENCH_ARGS < <(bench_args "${BENCH_NAME}" "${ITERATIONS}" "${SCENARIO_NAME}")
    BENCH_BIN="${BUILD_DIR}/bench/${BENCH_NAME}"
    exec valgrind --tool=callgrind \
        --callgrind-out-file="${RESULTS_DIR}/callgrind-${BENCH_NAME}-${SCENARIO_NAME}.out" \
        "${BENCH_BIN}" "${BENCH_ARGS[@]}"
    ;;
gdb)
    require_perf_lane
    need_cmd gdb
    ensure_perf_build
    SCENARIO_NAME="$(normalize_scenario "${BENCH_NAME}" "${SCENARIO}")"
    mapfile -t BENCH_ARGS < <(bench_args "${BENCH_NAME}" "${ITERATIONS}" "${SCENARIO_NAME}")
    BENCH_BIN="${BUILD_DIR}/bench/${BENCH_NAME}"
    exec gdb --args "${BENCH_BIN}" "${BENCH_ARGS[@]}"
    ;;
hyperfine)
    need_cmd hyperfine
    ensure_perf_build
    SCENARIO_NAME="$(normalize_scenario "${BENCH_NAME}" "${SCENARIO}")"
    mapfile -t BENCH_ARGS < <(bench_args "${BENCH_NAME}" "${ITERATIONS}" "${SCENARIO_NAME}")
    BENCH_BIN="${BUILD_DIR}/bench/${BENCH_NAME}"
    exec hyperfine \
        --warmup 3 \
        --export-markdown "${RESULTS_DIR}/hyperfine-${BENCH_NAME}-${SCENARIO_NAME}.md" \
        "$(command_string "${BENCH_BIN}" "${BENCH_ARGS[@]}")"
    ;;
*)
    usage
    exit 2
    ;;
esac
