#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUN_ID="${RUN_ID:-$(date +%Y%m%d-%H%M%S)}"
RESULTS_ROOT="${RESULTS_ROOT:-$ROOT_DIR/docs/tmp/benchmarks}"
RUN_DIR="${RESULTS_ROOT}/${RUN_ID}"
TIER1_ROOT="${TIER1_ROOT:-$ROOT_DIR/build/tier1}"
SRC_ROOT="${TIER1_ROOT}/src"
BUILD_ROOT="${TIER1_ROOT}/build"
BIN_ROOT="${TIER1_ROOT}/bin"
LIB_ROOT="${TIER1_ROOT}/lib"
HOT_ITERATIONS="${HOT_ITERATIONS:-50000}"

mkdir -p "${RUN_DIR}" "${BUILD_ROOT}" "${BIN_ROOT}" "${LIB_ROOT}"

if [[ ! -d "${SRC_ROOT}/zlog" || ! -d "${SRC_ROOT}/stumpless" || ! -d "${SRC_ROOT}/tinylog" ]]; then
    echo "Tier 1 source trees are missing under ${SRC_ROOT}" >&2
    echo "Clone them first before running this script." >&2
    exit 2
fi

cmake -S "${SRC_ROOT}/zlog" -B "${BUILD_ROOT}/zlog-upstream" -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build "${BUILD_ROOT}/zlog-upstream" -j4 >/dev/null

cmake -S "${SRC_ROOT}/stumpless" -B "${BUILD_ROOT}/stumpless-upstream" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON \
    -DBUILD_BENCHMARKING=OFF \
    -DINSTALL_HTML=OFF \
    -DINSTALL_MANPAGES=OFF \
    -DINSTALL_EXAMPLES=OFF >/dev/null
cmake --build "${BUILD_ROOT}/stumpless-upstream" -j4 --target stumpless >/dev/null

cmake -S "${SRC_ROOT}/tinylog" -B "${BUILD_ROOT}/tinylog-upstream" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 >/dev/null
cmake --build "${BUILD_ROOT}/tinylog-upstream" -j4 >/dev/null

cp "${BUILD_ROOT}/zlog-upstream/lib/libzlog.so" "${LIB_ROOT}/"
cp "${BUILD_ROOT}/stumpless-upstream/libstumpless.so" "${LIB_ROOT}/"
cp "${BUILD_ROOT}/tinylog-upstream/libtlog.so" "${LIB_ROOT}/"

export LD_LIBRARY_PATH="${LIB_ROOT}:${BUILD_ROOT}/zlog-upstream/lib:${BUILD_ROOT}/stumpless-upstream:${BUILD_ROOT}/tinylog-upstream${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

COMMON_CFLAGS=(-std=c23 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic)

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
