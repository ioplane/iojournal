#!/usr/bin/env bash
# shellcheck shell=bash
# Captures host/toolchain info and runs the release gate for RC verification.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd -P)"
readonly ROOT_DIR

RUNSTAMP="$(date -u +%Y%m%dT%H%M%SZ)"
readonly RUNSTAMP

HEAD_SHORT="$(git -C "${ROOT_DIR}" rev-parse --short HEAD 2>/dev/null || printf 'nogit')"
readonly HEAD_SHORT

readonly RUN_ID="${RUNSTAMP}-${HEAD_SHORT}"
readonly OUT_BASE="${ROOT_DIR}/dist/release-candidate"
readonly OUT_DIR="${OUT_BASE}/runs/${RUN_ID}"
readonly LATEST_FILE="${OUT_BASE}/latest.txt"
readonly INDEX_FILE="${OUT_BASE}/index.tsv"

mkdir -p "${OUT_DIR}"

{
    printf 'run_id\t%s\n' "${RUN_ID}"
    printf 'git_head_short\t%s\n' "${HEAD_SHORT}"
    printf 'utc_started\t%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "${OUT_DIR}/run.txt"

{
    uname -a
    printf '%s\n' "---"
    lscpu | sed -n '1,80p'
} > "${OUT_DIR}/host.txt"

{
    clang --version | sed -n '1,2p' || true
    printf '%s\n' "---"
    cmake --version | sed -n '1,1p' || true
    printf '%s\n' "---"
    python3 --version
} > "${OUT_DIR}/toolchain.txt"

bash "${ROOT_DIR}/scripts/run-release-gate.sh" > "${OUT_DIR}/release-gate.txt" 2>&1

cat > "${OUT_DIR}/summary.md" <<EOF
# Release Candidate Verification Summary

Run id: \`${RUN_ID}\`

## Verification Scope

- release gate
- Sprint 10 functional and performance comparison evidence
- Sprint 11A and Sprint 11B optimization evidence
- Sprint 12 publication checklist and decision surface
- host and toolchain capture
- bootstrap-state artifact generation

## Published Files

- \`run.txt\`
- \`host.txt\`
- \`toolchain.txt\`
- \`release-gate.txt\`
- \`summary.md\`

## Evidence References

- \`docs/plans/comparison/PERFORMANCE_RESULTS.md\`
- \`docs/en/05-release-candidate-checklist.md\`
- \`docs/en/06-rc1-publish-decision.md\`

## Result

Local release-candidate checks completed for this run.
EOF

if [[ ! -f "${INDEX_FILE}" ]]; then
    printf 'run_id\tgit_head_short\tstatus\n' > "${INDEX_FILE}"
fi
printf '%s\t%s\tPASS\n' "${RUN_ID}" "${HEAD_SHORT}" >> "${INDEX_FILE}"
printf '%s\n' "${RUN_ID}" > "${LATEST_FILE}"
