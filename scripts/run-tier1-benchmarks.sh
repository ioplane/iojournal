#!/usr/bin/env bash
# shellcheck shell=bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
if [[ -f /usr/local/lib/ioplane/common.sh ]]; then
    # shellcheck source=/dev/null
    source /usr/local/lib/ioplane/common.sh
else
    # shellcheck source=lib/common.sh disable=SC1091
    source "${SCRIPT_DIR}/lib/common.sh"
fi

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
readonly ROOT_DIR
RUN_ID="${RUN_ID:-$(date +%Y%m%d-%H%M%S)}"
readonly RUN_ID
readonly RESULTS_ROOT="${RESULTS_ROOT:-${ROOT_DIR}/docs/tmp/benchmarks}"
readonly RUN_DIR="${RESULTS_ROOT}/${RUN_ID}"
readonly TIER1_ROOT="${TIER1_ROOT:-${ROOT_DIR}/build/tier1}"
readonly SRC_ROOT="${TIER1_ROOT}/src"
readonly BUILD_ROOT="${TIER1_ROOT}/build"
readonly BIN_ROOT="${TIER1_ROOT}/bin"
readonly LIB_ROOT="${TIER1_ROOT}/lib"
readonly HOT_ITERATIONS="${HOT_ITERATIONS:-50000}"

mkdir -p "${RUN_DIR}" "${BUILD_ROOT}" "${BIN_ROOT}" "${LIB_ROOT}"

if [[ ! -d "${SRC_ROOT}/zlog" || ! -d "${SRC_ROOT}/stumpless" || ! -d "${SRC_ROOT}/tinylog" ]]; then
    printf "Tier 1 source trees are missing under %s\n" "${SRC_ROOT}" >&2
    printf "Clone them first before running this script.\n" >&2
    exit 2
fi

cmake -S "${SRC_ROOT}/zlog" -B "${BUILD_ROOT}/zlog-upstream" -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build "${BUILD_ROOT}/zlog-upstream" -j"$(nproc)" >/dev/null

cmake -S "${SRC_ROOT}/stumpless" -B "${BUILD_ROOT}/stumpless-upstream" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON \
    -DBUILD_BENCHMARKING=OFF \
    -DINSTALL_HTML=OFF \
    -DINSTALL_MANPAGES=OFF \
    -DINSTALL_EXAMPLES=OFF >/dev/null
cmake --build "${BUILD_ROOT}/stumpless-upstream" -j"$(nproc)" --target stumpless >/dev/null

cmake -S "${SRC_ROOT}/tinylog" -B "${BUILD_ROOT}/tinylog-upstream" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 >/dev/null
cmake --build "${BUILD_ROOT}/tinylog-upstream" -j"$(nproc)" >/dev/null

cp "${BUILD_ROOT}/zlog-upstream/lib/libzlog.so" "${LIB_ROOT}/"
cp "${BUILD_ROOT}/stumpless-upstream/libstumpless.so" "${LIB_ROOT}/"
cp "${BUILD_ROOT}/tinylog-upstream/libtlog.so" "${LIB_ROOT}/"

export LD_LIBRARY_PATH="${LIB_ROOT}:${BUILD_ROOT}/zlog-upstream/lib:${BUILD_ROOT}/stumpless-upstream:${BUILD_ROOT}/tinylog-upstream${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

readonly COMMON_CFLAGS=(-std=c23 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic)

cc "${COMMON_CFLAGS[@]}" -I"${ROOT_DIR}/bench/tier1" -I"${SRC_ROOT}/zlog/src" \
    "${ROOT_DIR}/bench/tier1/bench_zlog.c" -L"${LIB_ROOT}" -Wl,-rpath,"${LIB_ROOT}" -lzlog \
    -lpthread -o "${BIN_ROOT}/bench_zlog"

cc "${COMMON_CFLAGS[@]}" -I"${ROOT_DIR}/bench/tier1" \
    -I"${SRC_ROOT}/stumpless/include" -I"${BUILD_ROOT}/stumpless-upstream/include" \
    "${ROOT_DIR}/bench/tier1/bench_stumpless.c" -L"${LIB_ROOT}" \
    -Wl,-rpath,"${LIB_ROOT}" -lstumpless -lpthread -o "${BIN_ROOT}/bench_stumpless"

cc "${COMMON_CFLAGS[@]}" -I"${ROOT_DIR}/bench/tier1" -I"${SRC_ROOT}/tinylog" \
    "${ROOT_DIR}/bench/tier1/bench_tinylog.c" -L"${LIB_ROOT}" -Wl,-rpath,"${LIB_ROOT}" -ltlog \
    -lpthread -o "${BIN_ROOT}/bench_tinylog"

"${BIN_ROOT}/bench_zlog" "${HOT_ITERATIONS}" --tsv | tee "${RUN_DIR}/bench_zlog.tsv"
"${BIN_ROOT}/bench_stumpless" "${HOT_ITERATIONS}" --tsv | tee "${RUN_DIR}/bench_stumpless.tsv"
"${BIN_ROOT}/bench_tinylog" "${HOT_ITERATIONS}" --tsv | tee "${RUN_DIR}/bench_tinylog.tsv"

# shellcheck disable=SC2016
cat >"${RUN_DIR}/tier1-manifest.md" <<EOF
# Tier 1 Benchmark Manifest

| Field | Value |
| --- | --- |
| run_id | \`${RUN_ID}\` |
| shared_iterations | \`${HOT_ITERATIONS}\` |
| libraries | \`zlog\`, \`stumpless\`, \`tinylog\` |
| benchmark_scope | shared Tier 1 scenarios plus normalized file append |

## Artifacts

- \`bench_zlog.tsv\`
- \`bench_stumpless.tsv\`
- \`bench_tinylog.tsv\`
EOF

printf '%s\n' "${RUN_DIR}"
