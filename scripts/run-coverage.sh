#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/clang-coverage"
REPORT_DIR="${BUILD_DIR}/coverage"
PROFILE_DIR="${BUILD_DIR}/profiles"

mkdir -p "${REPORT_DIR}"

if [[ ! -f "${ROOT_DIR}/CMakeLists.txt" || ! -f "${ROOT_DIR}/CMakePresets.json" ]]; then
    printf "SKIP: coverage unavailable until the CMake project is initialized\n" > "${REPORT_DIR}/coverage.txt"
    printf "TN:\n" > "${REPORT_DIR}/coverage.lcov"
    chmod 0644 "${REPORT_DIR}/coverage.lcov" "${REPORT_DIR}/coverage.txt"
    cat "${REPORT_DIR}/coverage.txt"
    exit 0
fi

cmake --preset clang-coverage
cmake --build "${BUILD_DIR}" -j"$(nproc)"

rm -rf "${PROFILE_DIR}"
mkdir -p "${PROFILE_DIR}"

LLVM_PROFILE_FILE="${PROFILE_DIR}/%p.profraw" \
  ctest --test-dir "${BUILD_DIR}" --output-on-failure

llvm-profdata merge \
  -sparse \
  "${PROFILE_DIR}"/*.profraw \
  -o "${REPORT_DIR}/coverage.profdata"

mapfile -t COVERAGE_OBJECTS < <(find "${BUILD_DIR}" -maxdepth 1 -type f -perm -111 -name 'test_*' | sort)

if [[ ${#COVERAGE_OBJECTS[@]} -eq 0 ]]; then
  printf "SKIP: no test coverage objects found in %s\n" "${BUILD_DIR}" > "${REPORT_DIR}/coverage.txt"
  printf "TN:\n" > "${REPORT_DIR}/coverage.lcov"
  chmod 0644 "${REPORT_DIR}/coverage.lcov" "${REPORT_DIR}/coverage.txt"
  cat "${REPORT_DIR}/coverage.txt"
  exit 0
fi

LLVM_COV_OBJECT_ARGS=()
for obj in "${COVERAGE_OBJECTS[@]:1}"; do
  LLVM_COV_OBJECT_ARGS+=("-object=${obj}")
done

IGNORE_REGEX='(.*/tests/.*|.*/tests/third_party/.*|.*/usr/local/src/unity/.*)'

llvm-cov export \
  -format=lcov \
  "${COVERAGE_OBJECTS[0]}" \
  "${LLVM_COV_OBJECT_ARGS[@]}" \
  -instr-profile="${REPORT_DIR}/coverage.profdata" \
  -ignore-filename-regex="${IGNORE_REGEX}" \
  > "${REPORT_DIR}/coverage.lcov"

llvm-cov report \
  "${COVERAGE_OBJECTS[0]}" \
  "${LLVM_COV_OBJECT_ARGS[@]}" \
  -instr-profile="${REPORT_DIR}/coverage.profdata" \
  -ignore-filename-regex="${IGNORE_REGEX}" \
  > "${REPORT_DIR}/coverage.txt"

cat "${REPORT_DIR}/coverage.txt"
chmod 0644 "${REPORT_DIR}/coverage.lcov" "${REPORT_DIR}/coverage.txt"
