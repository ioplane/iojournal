#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_ROOT="${SRC_ROOT:-$ROOT_DIR/docs/tmp/competitors/src}"
BUILD_ROOT="${BUILD_ROOT:-$ROOT_DIR/build/tier1/build}"
INSTALL_ROOT="${INSTALL_ROOT:-$ROOT_DIR/build/tier1/install}"

bash "${ROOT_DIR}/scripts/fetch-tier1-competitors.sh"

mkdir -p "${BUILD_ROOT}" "${INSTALL_ROOT}"

build_zlog() {
    local src="${SRC_ROOT}/zlog"
    local build="${BUILD_ROOT}/zlog"
    local install="${INSTALL_ROOT}/zlog"

    cmake --fresh -S "${src}" -B "${build}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="${install}"
    cmake --build "${build}" --target zlog zlog_s zlog-chk-conf
    cmake --install "${build}"
}

build_stumpless() {
    local src="${SRC_ROOT}/stumpless"
    local build="${BUILD_ROOT}/stumpless"
    local install="${INSTALL_ROOT}/stumpless"
    local compat="${BUILD_ROOT}/stumpless-compat.cmake"

    cat >"${compat}" <<'EOF'
if(NOT COMMAND add_function_test)
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
EOF

    cmake --fresh -S "${src}" -B "${build}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="${install}" \
        -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="${compat}" \
        -DBUILD_TESTING=OFF \
        -DBUILD_BENCHMARKING=OFF \
        -DBUILD_CPP=OFF \
        -DBUILD_PYTHON=OFF \
        -DINSTALL_HTML=OFF \
        -DINSTALL_MANPAGES=OFF \
        -DINSTALL_EXAMPLES=OFF
    cmake --build "${build}" --target stumpless
    cmake --install "${build}"
}

build_tinylog() {
    local src="${SRC_ROOT}/tinylog"
    local build="${BUILD_ROOT}/tinylog"
    local install="${INSTALL_ROOT}/tinylog"

    cmake --fresh -S "${src}" -B "${build}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="${install}" \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    cmake --build "${build}" --target tlog
    cmake --install "${build}"
}

build_zlog
build_stumpless
build_tinylog

{
    printf '# Tier 1 Competitor Install Roots\n\n'
    printf '| Library | Install Prefix |\n'
    printf '| --- | --- |\n'
    printf '| `zlog` | `%s` |\n' "${INSTALL_ROOT}/zlog"
    printf '| `stumpless` | `%s` |\n' "${INSTALL_ROOT}/stumpless"
    printf '| `tinylog` | `%s` |\n' "${INSTALL_ROOT}/tinylog"
} >"${INSTALL_ROOT}/README.md"

printf '%s\n' "${INSTALL_ROOT}"
