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
readonly COVERAGE_TOOLCHAIN="${COVERAGE_TOOLCHAIN:-all}"
readonly CLANG_BUILD_DIR="${ROOT_DIR}/build/clang-coverage"
readonly CLANG_REPORT_DIR="${CLANG_BUILD_DIR}/coverage"
readonly CLANG_PROFILE_DIR="${CLANG_BUILD_DIR}/profiles"
readonly GCC_BUILD_DIR="${ROOT_DIR}/build/gcc-coverage"
readonly GCC_REPORT_DIR="${GCC_BUILD_DIR}/coverage"

mkdir -p "${CLANG_REPORT_DIR}" "${GCC_REPORT_DIR}"

if [[ ! -f "${ROOT_DIR}/CMakeLists.txt" || ! -f "${ROOT_DIR}/CMakePresets.json" ]]; then
    printf "SKIP: coverage unavailable until the CMake project is initialized\n" > "${CLANG_REPORT_DIR}/coverage.txt"
    printf "TN:\n" > "${CLANG_REPORT_DIR}/coverage.lcov"
    printf "SKIP: coverage unavailable until the CMake project is initialized\n" > "${GCC_REPORT_DIR}/coverage.txt"
    printf "TN:\n" > "${GCC_REPORT_DIR}/coverage.lcov"
    chmod 0644 \
        "${CLANG_REPORT_DIR}/coverage.lcov" \
        "${CLANG_REPORT_DIR}/coverage.txt" \
        "${GCC_REPORT_DIR}/coverage.lcov" \
        "${GCC_REPORT_DIR}/coverage.txt"
    cat "${CLANG_REPORT_DIR}/coverage.txt"
    exit 0
fi

run_clang_coverage() {
    local ignore_regex
    local object_args=()
    local coverage_objects

    cmake --preset clang-coverage
    cmake --build "${CLANG_BUILD_DIR}" -j"$(nproc)"

    rm -rf "${CLANG_PROFILE_DIR}"
    mkdir -p "${CLANG_PROFILE_DIR}"

    LLVM_PROFILE_FILE="${CLANG_PROFILE_DIR}/%p.profraw" \
      ctest --test-dir "${CLANG_BUILD_DIR}" --output-on-failure

    llvm-profdata merge \
      -sparse \
      "${CLANG_PROFILE_DIR}"/*.profraw \
      -o "${CLANG_REPORT_DIR}/coverage.profdata"

    mapfile -t coverage_objects < <(find "${CLANG_BUILD_DIR}" -type f -perm -111 -name 'test_*' | sort)

    if [[ ${#coverage_objects[@]} -eq 0 ]]; then
      printf "SKIP: no test coverage objects found in %s\n" "${CLANG_BUILD_DIR}" > "${CLANG_REPORT_DIR}/coverage.txt"
      printf "TN:\n" > "${CLANG_REPORT_DIR}/coverage.lcov"
      chmod 0644 "${CLANG_REPORT_DIR}/coverage.lcov" "${CLANG_REPORT_DIR}/coverage.txt"
      cat "${CLANG_REPORT_DIR}/coverage.txt"
      return 0
    fi

    for obj in "${coverage_objects[@]:1}"; do
      object_args+=("-object=${obj}")
    done

    ignore_regex='(.*/tests/.*|.*/tests/third_party/.*|.*/usr/local/src/unity/.*)'

    llvm-cov export \
      -format=lcov \
      "${coverage_objects[0]}" \
      "${object_args[@]}" \
      -instr-profile="${CLANG_REPORT_DIR}/coverage.profdata" \
      -ignore-filename-regex="${ignore_regex}" \
      > "${CLANG_REPORT_DIR}/coverage.lcov"

    llvm-cov report \
      "${coverage_objects[0]}" \
      "${object_args[@]}" \
      -instr-profile="${CLANG_REPORT_DIR}/coverage.profdata" \
      -ignore-filename-regex="${ignore_regex}" \
      > "${CLANG_REPORT_DIR}/coverage.txt"

    chmod 0644 "${CLANG_REPORT_DIR}/coverage.lcov" "${CLANG_REPORT_DIR}/coverage.txt"
}

run_gcc_coverage() {
    if ! command -v gcovr >/dev/null 2>&1; then
        printf "SKIP: gcovr is not available in the current environment\n" > "${GCC_REPORT_DIR}/coverage.txt"
        printf "TN:\n" > "${GCC_REPORT_DIR}/coverage.lcov"
        chmod 0644 "${GCC_REPORT_DIR}/coverage.lcov" "${GCC_REPORT_DIR}/coverage.txt"
        cat "${GCC_REPORT_DIR}/coverage.txt"
        return 0
    fi

    cmake --preset gcc-coverage
    cmake --build "${GCC_BUILD_DIR}" -j"$(nproc)"
    ctest --test-dir "${GCC_BUILD_DIR}" --output-on-failure

    gcovr \
        --root "${ROOT_DIR}" \
        --filter "${ROOT_DIR}/src" \
        --exclude "${ROOT_DIR}/tests/.*" \
        --exclude "${ROOT_DIR}/tests/third_party/.*" \
        --txt-summary \
        --txt "${GCC_REPORT_DIR}/coverage.txt" \
        --lcov "${GCC_REPORT_DIR}/coverage.lcov" \
        --html-details "${GCC_REPORT_DIR}/coverage.html" \
        "${GCC_BUILD_DIR}"

    chmod 0644 "${GCC_REPORT_DIR}/coverage.lcov" "${GCC_REPORT_DIR}/coverage.txt" \
        "${GCC_REPORT_DIR}/coverage.html"
}

case "${COVERAGE_TOOLCHAIN}" in
clang)
    run_clang_coverage
    cat "${CLANG_REPORT_DIR}/coverage.txt"
    ;;
gcc)
    run_gcc_coverage
    cat "${GCC_REPORT_DIR}/coverage.txt"
    ;;
all)
    run_clang_coverage
    run_gcc_coverage
    printf "\n--- clang coverage ---\n"
    cat "${CLANG_REPORT_DIR}/coverage.txt"
    printf "\n--- gcc coverage ---\n"
    cat "${GCC_REPORT_DIR}/coverage.txt"
    ;;
*)
    printf "unknown COVERAGE_TOOLCHAIN: %s\n" "${COVERAGE_TOOLCHAIN}" >&2
    exit 2
    ;;
esac
