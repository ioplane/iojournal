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
readonly SRC_ROOT="${SRC_ROOT:-${ROOT_DIR}/docs/tmp/competitors/src}"
readonly BUILD_ROOT="${BUILD_ROOT:-${ROOT_DIR}/build/tier1/build}"
readonly INSTALL_ROOT="${INSTALL_ROOT:-${ROOT_DIR}/build/tier1/install}"

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

# shellcheck disable=SC2016
{
    printf '# Tier 1 Competitor Install Roots\n\n'
    printf '| Library | Install Prefix |\n'
    printf '| --- | --- |\n'
    printf '| `zlog` | `%s` |\n' "${INSTALL_ROOT}/zlog"
    printf '| `stumpless` | `%s` |\n' "${INSTALL_ROOT}/stumpless"
    printf '| `tinylog` | `%s` |\n' "${INSTALL_ROOT}/tinylog"
} >"${INSTALL_ROOT}/README.md"

printf '%s\n' "${INSTALL_ROOT}"
