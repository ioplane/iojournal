#!/usr/bin/env bash
# shellcheck shell=bash
# Generates dist/RELEASE_NOTES.md for the given tag.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
readonly ROOT_DIR

readonly DIST_DIR="${ROOT_DIR}/dist"
readonly TAG_NAME="${1:-${GITHUB_REF_NAME:-dev}}"

RC_RUN_ID="bootstrap-not-run"
if [[ -f "${ROOT_DIR}/dist/release-candidate/latest.txt" ]]; then
    RC_RUN_ID="$(cat "${ROOT_DIR}/dist/release-candidate/latest.txt")"
fi
readonly RC_RUN_ID

mkdir -p "${DIST_DIR}"

cat > "${DIST_DIR}/RELEASE_NOTES.md" <<EOF
# iojournal ${TAG_NAME}

## Scope

- bounded logging library for the \`io*\` ecosystem
- synchronous RC API with stable console, file, and syslog sinks
- release-blocking comparison and optimization evidence included in the RC decision surface

## Verification

- release gate: \`scripts/run-release-gate.sh\`
- release-candidate evidence: \`${RC_RUN_ID}\`
- comparison evidence: \`docs/plans/comparison/PERFORMANCE_RESULTS.md\`
- publish decision: \`docs/en/06-rc1-publish-decision.md\`

## Published Assets

- source tarball
- source zip archive when \`zip\` is available
- generated API reference archive when docs have been built
- verification artifact archive when release-candidate output exists
- checksums

## References

- roadmap: \`docs/plans/ROADMAP.md\`
- release checklist: \`docs/en/05-release-candidate-checklist.md\`
- publish decision: \`docs/en/06-rc1-publish-decision.md\`
- local RFC mirror: \`docs/rfc/\`
EOF
