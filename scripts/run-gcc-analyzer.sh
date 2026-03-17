#!/usr/bin/env bash
# shellcheck shell=bash
# Runs the GCC -fanalyzer lane and reports warnings.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

# shellcheck source=lib/common.sh disable=SC1091
if [[ -f /usr/local/lib/ioplane/common.sh ]]; then
    source /usr/local/lib/ioplane/common.sh
else
    source "${SCRIPT_DIR}/lib/common.sh"
fi

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
readonly ROOT_DIR

readonly PRESET="${PRESET:-gcc-analyzer}"
readonly BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/${PRESET}}"
readonly REPORT_DIR="${BUILD_DIR}/analyzer"

mkdir -p "${REPORT_DIR}"

if [[ ! -f "${ROOT_DIR}/CMakeLists.txt" || ! -f "${ROOT_DIR}/CMakePresets.json" ]]; then
    printf 'SKIP: GCC analyzer unavailable until the CMake project is initialized\n' \
        > "${REPORT_DIR}/summary.txt"
    cat "${REPORT_DIR}/summary.txt"
    exit 0
fi

if ! command -v gcc >/dev/null 2>&1; then
    printf 'SKIP: gcc is not available in the current environment\n' > "${REPORT_DIR}/summary.txt"
    cat "${REPORT_DIR}/summary.txt"
    exit 0
fi

cmake --preset "${PRESET}"
cmake --build --preset "${PRESET}" 2>&1 | tee "${REPORT_DIR}/build.log"
ctest --preset "${PRESET}" --output-on-failure 2>&1 | tee "${REPORT_DIR}/tests.log"

gcc --version | head -n 1 > "${REPORT_DIR}/gcc-version.txt"
gcc -Q --help=optimizers > "${REPORT_DIR}/gcc-optimizers.txt" 2>/dev/null || true

ALL_WARNING_LINES="$(grep 'warning:' "${REPORT_DIR}/build.log" || true)"
readonly ALL_WARNING_LINES

KNOWN_WARNING_LINES="$(printf '%s\n' "${ALL_WARNING_LINES}" | grep -E 'analyzer-too-complex|analyzer-symbol-too-complex|analyzer-malloc-leak|analyzer-fd-leak|format-truncation|format-overflow' || true)"
readonly KNOWN_WARNING_LINES

OTHER_WARNING_LINES="$(printf '%s\n' "${ALL_WARNING_LINES}" | grep -Ev 'analyzer-too-complex|analyzer-symbol-too-complex|analyzer-malloc-leak|analyzer-fd-leak|format-truncation|format-overflow' || true)"
readonly OTHER_WARNING_LINES

TOTAL_WARNINGS="$(printf '%s\n' "${ALL_WARNING_LINES}" | grep -c 'warning:' || true)"
readonly TOTAL_WARNINGS

KNOWN_WARNINGS="$(printf '%s\n' "${KNOWN_WARNING_LINES}" | grep -c 'warning:' || true)"
readonly KNOWN_WARNINGS

OTHER_WARNINGS="$(printf '%s\n' "${OTHER_WARNING_LINES}" | grep -c 'warning:' || true)"
readonly OTHER_WARNINGS

cat > "${REPORT_DIR}/summary.txt" <<EOF
PASS: gcc analyzer lane completed
PRESET=${PRESET}
BUILD_DIR=${BUILD_DIR}
TOTAL_WARNINGS=${TOTAL_WARNINGS}
KNOWN_WARNINGS=${KNOWN_WARNINGS}
OTHER_WARNINGS=${OTHER_WARNINGS}
EOF

if [[ "${OTHER_WARNINGS}" -gt 0 ]]; then
    printf '%s\n' "${OTHER_WARNING_LINES}" > "${REPORT_DIR}/unexpected-warnings.txt"
    cat "${REPORT_DIR}/summary.txt"
    cat "${REPORT_DIR}/unexpected-warnings.txt"
    exit 1
fi

printf '%s\n' "${KNOWN_WARNING_LINES}" > "${REPORT_DIR}/known-warnings.txt"
cat "${REPORT_DIR}/summary.txt"
