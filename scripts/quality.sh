#!/usr/bin/env bash
# Repository quality pipeline for iojournal.
# Run inside the dev container. PVS credentials are loaded from the
# environment when available; do not bake them into the image.
set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m'

PASS=0
FAIL=0
SKIP=0

step() { printf "\n${CYAN}=== [%d/11] %s ===${NC}\n" "$1" "$2"; }
ok()   { printf "${GREEN}PASS${NC}: %s\n" "$1"; PASS=$((PASS + 1)); }
fail() { printf "${RED}FAIL${NC}: %s\n" "$1"; FAIL=$((FAIL + 1)); }
skip() { printf "${YELLOW}SKIP${NC}: %s\n" "$1"; SKIP=$((SKIP + 1)); }

has_cmake_surface() {
    [[ -f CMakeLists.txt && -f CMakePresets.json ]]
}

BUILD_DIR="${BUILD_DIR:-build/clang-debug}"
PRESET="${PRESET:-clang-debug}"
NPROC="$(nproc)"
ROOT_DIR="$(pwd -P)"
THIRD_PARTY_DIR="${ROOT_DIR}/tests/third_party"

step 1 "Repository baseline"
if [[ -d .github && -f AGENTS.md && -f CLAUDE.md && -f CODEX.md ]]; then
    ok "Repository baseline files present"
else
    fail "Missing baseline files (.github or root instructions)"
fi

step 2 "Stale parser identifier scan"
if rg -n 'iohttpparser|ihtp_|IHTP_|HTTP parser|scanner -> parser -> semantics' \
    --glob '!scripts/quality.sh' \
    .github scripts AGENTS.md CLAUDE.md CODEX.md >/dev/null 2>&1; then
    fail "Found stale parser-specific identifiers"
else
    ok "No stale parser-specific identifiers"
fi

step 3 "Documentation lint"
if [[ -f scripts/lint-docs.py ]]; then
    if python3 scripts/lint-docs.py; then
        ok "Documentation lint clean"
    else
        fail "Documentation lint failed"
    fi
else
    skip "Documentation linter not present"
fi

step 4 "Project bootstrap scripts"
if [[ -f scripts/run-release-gate.sh && -f scripts/run-coverage.sh && -f scripts/run-gcc-analyzer.sh && -f scripts/build-release-assets.sh ]]; then
    ok "Required bootstrap scripts present"
else
    fail "Missing bootstrap scripts"
fi

step 5 "Configure and build"
if has_cmake_surface; then
    if cmake --preset "${PRESET}" --fresh && \
       cmake --build --preset "${PRESET}"; then
        EXAMPLE_BIN="${BUILD_DIR}/examples/basic_console"
        FILE_EXAMPLE_BIN="${BUILD_DIR}/examples/file_sink"
        SYSLOG_UDP_EXAMPLE_BIN="${BUILD_DIR}/examples/syslog_udp"
        SYSLOG_TCP_EXAMPLE_BIN="${BUILD_DIR}/examples/syslog_tcp"
        if [[ -x "${EXAMPLE_BIN}" ]]; then
            EXAMPLE_OUT="$("${EXAMPLE_BIN}" 2>&1)" || {
                echo "${EXAMPLE_OUT}"
                fail "Build succeeded but example execution failed"
                EXAMPLE_OUT=""
            }
            if [[ -n "${EXAMPLE_OUT}" ]] && \
                echo "${EXAMPLE_OUT}" | grep -q '"event_name":"example.start"'; then
                ok "Build succeeded and example emitted console JSON"
            elif [[ -n "${EXAMPLE_OUT}" ]]; then
                echo "${EXAMPLE_OUT}"
                fail "Build succeeded but example output contract changed"
            fi
        elif [[ ! -x "${FILE_EXAMPLE_BIN}" ]]; then
            ok "Build succeeded"
        fi
        if [[ -x "${FILE_EXAMPLE_BIN}" ]]; then
            FILE_EXAMPLE_PATH="$("${FILE_EXAMPLE_BIN}" 2>/tmp/iojournal-file-example.err)" || {
                cat /tmp/iojournal-file-example.err
                fail "Build succeeded but file sink example execution failed"
                FILE_EXAMPLE_PATH=""
            }
            rm -f /tmp/iojournal-file-example.err
            if [[ -n "${FILE_EXAMPLE_PATH}" && -f "${FILE_EXAMPLE_PATH}" ]] && \
                grep -q '"event_name":"example.file"' "${FILE_EXAMPLE_PATH}" && \
                grep -q '"auth.token":"\[REDACTED\]"' "${FILE_EXAMPLE_PATH}"; then
                rm -f "${FILE_EXAMPLE_PATH}"
                ok "File sink example emitted NDJSON output"
            elif [[ -n "${FILE_EXAMPLE_PATH}" ]]; then
                if [[ -f "${FILE_EXAMPLE_PATH}" ]]; then
                    cat "${FILE_EXAMPLE_PATH}"
                    rm -f "${FILE_EXAMPLE_PATH}"
                fi
                fail "Build succeeded but file sink example output contract changed"
            fi
        fi
        if [[ -x "${SYSLOG_UDP_EXAMPLE_BIN}" ]]; then
            SYSLOG_UDP_OUT="$("${SYSLOG_UDP_EXAMPLE_BIN}" 2>&1)" || {
                echo "${SYSLOG_UDP_OUT}"
                fail "Build succeeded but syslog UDP example execution failed"
                SYSLOG_UDP_OUT=""
            }
            if [[ -n "${SYSLOG_UDP_OUT}" ]] && \
                echo "${SYSLOG_UDP_OUT}" | grep -q '^<166>1 ' && \
                echo "${SYSLOG_UDP_OUT}" | grep -q 'examples.syslog_udp - - - udp example payload'; then
                ok "Syslog UDP example emitted RFC 5424 datagram"
            elif [[ -n "${SYSLOG_UDP_OUT}" ]]; then
                echo "${SYSLOG_UDP_OUT}"
                fail "Build succeeded but syslog UDP example output contract changed"
            fi
        fi
        if [[ -x "${SYSLOG_TCP_EXAMPLE_BIN}" ]]; then
            SYSLOG_TCP_OUT="$("${SYSLOG_TCP_EXAMPLE_BIN}" 2>&1)" || {
                echo "${SYSLOG_TCP_OUT}"
                fail "Build succeeded but syslog TCP example execution failed"
                SYSLOG_TCP_OUT=""
            }
            if [[ -n "${SYSLOG_TCP_OUT}" ]] && \
                echo "${SYSLOG_TCP_OUT}" | grep -Eq '^[0-9]+ <139>1 ' && \
                echo "${SYSLOG_TCP_OUT}" | grep -q 'examples.syslog_tcp - - - tcp example payload'; then
                ok "Syslog TCP example emitted RFC 6587 frame"
            elif [[ -n "${SYSLOG_TCP_OUT}" ]]; then
                echo "${SYSLOG_TCP_OUT}"
                fail "Build succeeded but syslog TCP example output contract changed"
            fi
        fi
    else
        fail "Build failed"
    fi
else
    skip "CMake surface not initialized yet"
fi

step 6 "Unit tests"
if has_cmake_surface; then
    if ctest --preset "${PRESET}" --output-on-failure 2>&1; then
        ok "All tests passed"
    else
        fail "Some tests failed"
    fi
else
    skip "CMake surface not initialized yet"
fi

step 7 "Format check"
if has_cmake_surface; then
    if cmake --build --preset "${PRESET}" --target format-check 2>&1; then
        ok "Formatting clean"
    else
        fail "Formatting issues found"
    fi
else
    skip "Format target unavailable before CMake bootstrap"
fi

step 8 "cppcheck"
if has_cmake_surface; then
    if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
        fail "cppcheck: compile database missing"
    elif command -v cppcheck >/dev/null 2>&1; then
        if cppcheck --enable=warning,performance,portability \
            --error-exitcode=1 --inline-suppr \
            --project="${BUILD_DIR}/compile_commands.json" \
            --suppress='*:/usr/local/src/unity/*' \
            -q 2>&1; then
            ok "cppcheck clean"
        else
            fail "cppcheck found issues"
        fi
    else
        skip "cppcheck not installed"
    fi
else
    skip "Compile database unavailable before CMake bootstrap"
fi

step 9 "PVS-Studio"
if has_cmake_surface; then
    if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
        fail "PVS-Studio: compile database missing"
    elif command -v pvs-studio-analyzer >/dev/null 2>&1; then
        if [[ -z "${PVS_NAME:-}" || -z "${PVS_KEY:-}" ]]; then
            skip "PVS-Studio: missing PVS_NAME/PVS_KEY in environment"
        else
            pvs-studio-analyzer credentials "${PVS_NAME}" "${PVS_KEY}" >/dev/null 2>&1
            PVS_LOG="${BUILD_DIR}/pvs-studio.log"
            if pvs-studio-analyzer analyze \
                -f "${BUILD_DIR}/compile_commands.json" \
                -o "${PVS_LOG}" \
                -e /usr/local/src/unity/ \
                -j"${NPROC}" 2>&1 | grep -v '^\['; then
                PVS_OUT=$(plog-converter -t errorfile -a 'GA:1,2' "${PVS_LOG}" 2>/dev/null \
                    | grep -v '^pvs-studio.com' | grep -v '^Analyzer log' \
                    | grep -v '^PVS-Studio is' | grep -v '^$' \
                    | grep -v 'Total messages' | grep -v 'Filtered messages' \
                    | grep -v '^Copyright' \
                    | grep -v 'V1042' || true)
                PVS_COUNT=$(echo "${PVS_OUT}" | grep -cE '(error|warning):' || true)
                if [[ "${PVS_COUNT}" -eq 0 ]]; then
                    ok "PVS-Studio clean (GA:1,2)"
                else
                    echo "${PVS_OUT}"
                    fail "PVS-Studio: ${PVS_COUNT} errors/warnings"
                fi
            else
                fail "PVS-Studio analysis failed"
            fi
        fi
    else
        skip "PVS-Studio not installed"
    fi
else
    skip "Compile database unavailable before CMake bootstrap"
fi

step 10 "GCC analyzer"
if has_cmake_surface; then
    if [[ -f scripts/run-gcc-analyzer.sh ]]; then
        if bash scripts/run-gcc-analyzer.sh >/tmp/iojournal-gcc-analyzer.log 2>&1; then
            ok "GCC analyzer lane clean"
        else
            cat /tmp/iojournal-gcc-analyzer.log
            fail "GCC analyzer lane failed"
        fi
    else
        fail "GCC analyzer script missing"
    fi
else
    skip "Compile database unavailable before CMake bootstrap"
fi

step 11 "CodeChecker"
if has_cmake_surface; then
    if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
        fail "CodeChecker: compile database missing"
    elif command -v CodeChecker >/dev/null 2>&1; then
        CC_DIR=$(mktemp -d)
        CC_SKIP=$(mktemp)
        cat > "${CC_SKIP}" <<SKIP
-/usr/local/src/unity/*
-${THIRD_PARTY_DIR}/*
SKIP
        if CodeChecker analyze "${BUILD_DIR}/compile_commands.json" \
            -o "${CC_DIR}" \
            --analyzers clangsa clang-tidy \
            --skip "${CC_SKIP}" \
            -j"${NPROC}"; then
            CC_PARSE=$(CodeChecker parse "${CC_DIR}" \
                --trim-path-prefix "$(pwd)/" 2>&1 || true)
            CC_PARSE=$(echo "${CC_PARSE}" \
                | grep -v '^\[INFO\]' \
                | grep -v '^$' \
                | grep -v '/usr/local/src/unity/' \
                | grep -v "${THIRD_PARTY_DIR}/" || true)
            CC_HIGH=$(echo "${CC_PARSE}" | grep -c '\[HIGH\]' || true)
            CC_MED=$(echo "${CC_PARSE}" | grep -c '\[MEDIUM\]' || true)
            if [[ "${CC_HIGH}" -gt 0 || "${CC_MED}" -gt 0 ]]; then
                echo "${CC_PARSE}" | grep -E '\[(HIGH|MEDIUM)\]' || true
                fail "CodeChecker: ${CC_HIGH} HIGH, ${CC_MED} MEDIUM"
            else
                ok "CodeChecker clean (no HIGH/MEDIUM)"
            fi
        else
            fail "CodeChecker analysis failed"
        fi
        rm -rf "${CC_DIR}" "${CC_SKIP}"
    else
        skip "CodeChecker not installed"
    fi
else
    skip "Compile database unavailable before CMake bootstrap"
fi

printf "\n${CYAN}=== Summary ===${NC}\n"
printf "${GREEN}PASS: %d${NC}  ${RED}FAIL: %d${NC}  ${YELLOW}SKIP: %d${NC}\n" \
    "${PASS}" "${FAIL}" "${SKIP}"

if [[ "${FAIL}" -gt 0 ]]; then
    printf "${RED}Quality pipeline FAILED${NC}\n"
    exit 1
fi

printf "${GREEN}Quality pipeline PASSED${NC}\n"
