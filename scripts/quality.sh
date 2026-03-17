#!/usr/bin/env bash
# shellcheck shell=bash
# Repository quality pipeline for iojournal.
# Run inside the dev container. PVS credentials are loaded from the
# environment when available; do not bake them into the image.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

# Source shared ioplane library (container or local fallback)
# shellcheck disable=SC1091
if [[ -f /usr/local/lib/ioplane/common.sh ]]; then
    source /usr/local/lib/ioplane/common.sh
else
    source "${SCRIPT_DIR}/lib/common.sh"
fi

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd -P)"
readonly ROOT_DIR
readonly BUILD_DIR="${BUILD_DIR:-build/clang-debug}"
readonly PRESET="${PRESET:-clang-debug}"
readonly THIRD_PARTY_DIR="${ROOT_DIR}/tests/third_party"
readonly TOTAL_STEPS=12

cd "${ROOT_DIR}"

# ═══════════════════════════════════════════════════════
# Step 1: Repository baseline
# ═══════════════════════════════════════════════════════

ioj_step 1 "${TOTAL_STEPS}" "Repository baseline"
ioj_check_repo_baseline

# ═══════════════════════════════════════════════════════
# Step 2: Stale parser identifier scan
# ═══════════════════════════════════════════════════════

ioj_step 2 "${TOTAL_STEPS}" "Stale parser identifier scan"
if rg -n 'iohttpparser|ihtp_|IHTP_|HTTP parser|scanner -> parser -> semantics' \
    --glob '!scripts/quality.sh' \
    .github scripts AGENTS.md CLAUDE.md CODEX.md >/dev/null 2>&1; then
    ioj_record_fail "Found stale parser-specific identifiers"
else
    ioj_record_pass "No stale parser-specific identifiers"
fi

# ═══════════════════════════════════════════════════════
# Step 3: Documentation lint
# ═══════════════════════════════════════════════════════

ioj_step 3 "${TOTAL_STEPS}" "Documentation lint"
ioj_check_docs_lint

# ═══════════════════════════════════════════════════════
# Step 4: Project bootstrap scripts
# ═══════════════════════════════════════════════════════

ioj_step 4 "${TOTAL_STEPS}" "Project bootstrap scripts"
if [[ -f scripts/run-release-gate.sh && -f scripts/run-coverage.sh && -f scripts/run-gcc-analyzer.sh && -f scripts/build-release-assets.sh ]]; then
    ioj_record_pass "Required bootstrap scripts present"
else
    ioj_record_fail "Missing bootstrap scripts"
fi

# ═══════════════════════════════════════════════════════
# Step 5: Configure, build, and verify examples
# ═══════════════════════════════════════════════════════

ioj_step 5 "${TOTAL_STEPS}" "Configure and build"
if ioj_has_cmake_surface; then
    if cmake --preset "${PRESET}" --fresh && \
       cmake --build --preset "${PRESET}"; then

        # Console example
        if [[ -x "${BUILD_DIR}/examples/basic_console" ]]; then
            local_out=""
            local_out="$("${BUILD_DIR}/examples/basic_console" 2>&1)" || {
                printf '%s\n' "${local_out}"
                ioj_record_fail "Build succeeded but example execution failed"
                local_out=""
            }
            if [[ -n "${local_out}" ]] && \
                printf '%s\n' "${local_out}" | grep -q '"event_name":"example.start"'; then
                ioj_record_pass "Build succeeded and example emitted console JSON"
            elif [[ -n "${local_out}" ]]; then
                printf '%s\n' "${local_out}"
                ioj_record_fail "Build succeeded but example output contract changed"
            fi
        fi

        # File sink example
        if [[ -x "${BUILD_DIR}/examples/file_sink" ]]; then
            local_err="$(ioj_mktemp)"
            local_path=""
            local_path="$("${BUILD_DIR}/examples/file_sink" 2>"${local_err}")" || {
                cat "${local_err}"
                ioj_record_fail "Build succeeded but file sink example execution failed"
                local_path=""
            }
            if [[ -n "${local_path}" && -f "${local_path}" ]] && \
                grep -q '"event_name":"example.file"' "${local_path}" && \
                grep -q '"auth.token":"\[REDACTED\]"' "${local_path}"; then
                rm -f "${local_path}"
                ioj_record_pass "File sink example emitted NDJSON output"
            elif [[ -n "${local_path}" ]]; then
                if [[ -f "${local_path}" ]]; then
                    cat "${local_path}"
                    rm -f "${local_path}"
                fi
                ioj_record_fail "Build succeeded but file sink example output contract changed"
            fi
        fi

        # Syslog UDP example
        if [[ -x "${BUILD_DIR}/examples/syslog_udp" ]]; then
            local_out=""
            local_out="$("${BUILD_DIR}/examples/syslog_udp" 2>&1)" || {
                printf '%s\n' "${local_out}"
                ioj_record_fail "Build succeeded but syslog UDP example execution failed"
                local_out=""
            }
            if [[ -n "${local_out}" ]] && \
                printf '%s\n' "${local_out}" | grep -q '^<166>1 ' && \
                printf '%s\n' "${local_out}" | grep -q 'examples.syslog_udp - - - udp example payload'; then
                ioj_record_pass "Syslog UDP example emitted RFC 5424 datagram"
            elif [[ -n "${local_out}" ]]; then
                printf '%s\n' "${local_out}"
                ioj_record_fail "Build succeeded but syslog UDP example output contract changed"
            fi
        fi

        # Syslog TCP example
        if [[ -x "${BUILD_DIR}/examples/syslog_tcp" ]]; then
            local_out=""
            local_out="$("${BUILD_DIR}/examples/syslog_tcp" 2>&1)" || {
                printf '%s\n' "${local_out}"
                ioj_record_fail "Build succeeded but syslog TCP example execution failed"
                local_out=""
            }
            if [[ -n "${local_out}" ]] && \
                printf '%s\n' "${local_out}" | grep -Eq '^[0-9]+ <139>1 ' && \
                printf '%s\n' "${local_out}" | grep -q 'examples.syslog_tcp - - - tcp example payload'; then
                ioj_record_pass "Syslog TCP example emitted RFC 6587 frame"
            elif [[ -n "${local_out}" ]]; then
                printf '%s\n' "${local_out}"
                ioj_record_fail "Build succeeded but syslog TCP example output contract changed"
            fi
        fi
    else
        ioj_record_fail "Build failed"
    fi
else
    ioj_record_skip "CMake surface not initialized yet"
fi

# ═══════════════════════════════════════════════════════
# Step 6: Unit tests
# ═══════════════════════════════════════════════════════

ioj_step 6 "${TOTAL_STEPS}" "Unit tests"
if ioj_has_cmake_surface; then
    if ctest --preset "${PRESET}" --output-on-failure 2>&1; then
        ioj_record_pass "All tests passed"
    else
        ioj_record_fail "Some tests failed"
    fi
else
    ioj_record_skip "CMake surface not initialized yet"
fi

# ═══════════════════════════════════════════════════════
# Step 7: Format check
# ═══════════════════════════════════════════════════════

ioj_step 7 "${TOTAL_STEPS}" "Format check"
ioj_check_format "${PRESET}"

# ═══════════════════════════════════════════════════════
# Step 8: cppcheck
# ═══════════════════════════════════════════════════════

ioj_step 8 "${TOTAL_STEPS}" "cppcheck"
ioj_check_cppcheck "${BUILD_DIR}"

# ═══════════════════════════════════════════════════════
# Step 9: PVS-Studio
# ═══════════════════════════════════════════════════════

ioj_step 9 "${TOTAL_STEPS}" "PVS-Studio"
ioj_check_pvs_studio "${BUILD_DIR}"

# ═══════════════════════════════════════════════════════
# Step 10: GCC analyzer
# ═══════════════════════════════════════════════════════

ioj_step 10 "${TOTAL_STEPS}" "GCC analyzer"
ioj_check_gcc_analyzer

# ═══════════════════════════════════════════════════════
# Step 11: CodeChecker
# ═══════════════════════════════════════════════════════

ioj_step 11 "${TOTAL_STEPS}" "CodeChecker"
ioj_check_codechecker "${BUILD_DIR}" "${THIRD_PARTY_DIR}"

# ═══════════════════════════════════════════════════════
# Step 12: shellcheck
# ═══════════════════════════════════════════════════════

ioj_step 12 "${TOTAL_STEPS}" "Shellcheck"
ioj_check_shellcheck

# ═══════════════════════════════════════════════════════
# Summary
# ═══════════════════════════════════════════════════════

ioj_print_summary
