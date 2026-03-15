#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="${ROOT_DIR}/dist"
TAG_NAME="${1:-${GITHUB_REF_NAME:-dev}}"
RC_RUN_ID="bootstrap-not-run"

if [[ -f "${ROOT_DIR}/dist/release-candidate/latest.txt" ]]; then
  RC_RUN_ID="$(cat "${ROOT_DIR}/dist/release-candidate/latest.txt")"
fi

mkdir -p "${DIST_DIR}"

cat > "${DIST_DIR}/RELEASE_NOTES.md" <<EOF
# iojournal ${TAG_NAME}

## Scope

- structured logging library for the \`io*\` ecosystem
- bounded producer -> buffer -> filter -> encoder -> sink pipeline
- standards-oriented syslog, NDJSON, and OTLP-ready direction

## Verification

- release gate: \`scripts/run-release-gate.sh\`
- release-candidate evidence: \`${RC_RUN_ID}\`

## Published Assets

- source tarball
- source zip archive when \`zip\` is available
- generated API reference archive when docs have been built
- verification artifact archive when release-candidate output exists
- checksums

## References

- architecture plan: \`docs/plans/2026-03-10-iojournal-c23-architecture-plan.md\`
- repository bootstrap spec: \`docs/superpowers/specs/2026-03-14-iojournal-repository-bootstrap-design.md\`
- local RFC mirror: \`docs/rfc/\`
EOF
