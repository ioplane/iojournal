#!/usr/bin/env bash
# shellcheck shell=bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
readonly ROOT_DIR
readonly PRESET="${PRESET:-clang-debug}"
readonly BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/${PRESET}}"
readonly RESULTS_ROOT="${RESULTS_ROOT:-${ROOT_DIR}/docs/tmp/benchmarks}"
RUN_ID="${RUN_ID:-$(date +%Y%m%d-%H%M%S)}"
readonly RUN_ID
readonly RUN_DIR="${RESULTS_ROOT}/${RUN_ID}"
readonly HOT_ITERATIONS="${HOT_ITERATIONS:-50000}"
readonly FILE_ITERATIONS="${FILE_ITERATIONS:-${HOT_ITERATIONS}}"
readonly SYSLOG_ITERATIONS="${SYSLOG_ITERATIONS:-5000}"
readonly SCENARIO_STATUS_FILE="${RUN_DIR}/scenario-status.tsv"
readonly MANIFEST_FILE="${RUN_DIR}/manifest.md"

mkdir -p "${RUN_DIR}"

cmake --preset "${PRESET}"
cmake --build --preset "${PRESET}" --target bench_hot_path bench_file_sink bench_syslog_udp

"${BUILD_DIR}/bench/bench_hot_path" "${HOT_ITERATIONS}" --tsv \
    | tee "${RUN_DIR}/bench_hot_path.tsv"
"${BUILD_DIR}/bench/bench_file_sink" "${FILE_ITERATIONS}" --tsv \
    | tee "${RUN_DIR}/bench_file_sink.tsv"
"${BUILD_DIR}/bench/bench_syslog_udp" "${SYSLOG_ITERATIONS}" --tsv \
    | tee "${RUN_DIR}/bench_syslog_udp.tsv"

if command -v hyperfine >/dev/null 2>&1; then
    hyperfine \
        --warmup 2 \
        --export-markdown "${RUN_DIR}/hyperfine.md" \
        "\"${BUILD_DIR}/bench/bench_hot_path\" ${HOT_ITERATIONS} --tsv --scenario disabled_level" \
        "\"${BUILD_DIR}/bench/bench_hot_path\" ${HOT_ITERATIONS} --tsv --scenario enabled_console"
fi

cat >"${SCENARIO_STATUS_FILE}" <<EOF
scenario	class	status
disabled_level	shared	active
enabled_console	shared	active
append_ndjson	shared	active
udp_loopback	capability-specific	active
medium_message	shared	active
medium_message_with_metadata	shared	active
contention_mpsc	shared	active
EOF

# shellcheck disable=SC2016
{
    printf '# Benchmark Run Manifest\n\n'
    printf '| Field | Value |\n'
    printf '| --- | --- |\n'
    printf '| run_id | `%s` |\n' "${RUN_ID}"
    printf '| preset | `%s` |\n' "${PRESET}"
    printf '| build_dir | `%s` |\n' "${BUILD_DIR}"
    printf '| hot_iterations | `%s` |\n' "${HOT_ITERATIONS}"
    printf '| file_iterations | `%s` |\n' "${FILE_ITERATIONS}"
    printf '| syslog_iterations | `%s` |\n' "${SYSLOG_ITERATIONS}"
    printf '| hyperfine | `%s` |\n' "$(command -v hyperfine >/dev/null 2>&1 && printf 'present' || printf 'missing')"
    printf '\n## Artifacts\n\n'
    printf -- '- `bench_hot_path.tsv`\n'
    printf -- '- `bench_file_sink.tsv`\n'
    printf -- '- `bench_syslog_udp.tsv`\n'
    printf -- '- `scenario-status.tsv`\n'
    if [[ -f "${RUN_DIR}/hyperfine.md" ]]; then
        printf -- '- `hyperfine.md`\n'
    fi
    printf '\n## Scenario Status\n\n'
    printf '| Scenario | Class | Status |\n'
    printf '| --- | --- | --- |\n'
    while IFS=$'\t' read -r scenario class status; do
        if [[ "${scenario}" == "scenario" ]]; then
            continue
        fi
        printf '| `%s` | `%s` | `%s` |\n' "${scenario}" "${class}" "${status}"
    done <"${SCENARIO_STATUS_FILE}"
} >"${MANIFEST_FILE}"

for artifact in \
    "${RUN_DIR}/bench_hot_path.tsv" \
    "${RUN_DIR}/bench_file_sink.tsv" \
    "${RUN_DIR}/bench_syslog_udp.tsv"; do
    # shellcheck disable=SC2016
    grep -q $'^format\ttsv\tv1$' "${artifact}"
    # shellcheck disable=SC2016
    grep -q $'^columns\tbenchmark\tscenario\titerations\telapsed_ns\tns_per_op$' "${artifact}"
done

printf '%s\n' "${RUN_DIR}"
