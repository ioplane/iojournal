#!/usr/bin/env bash
# shellcheck shell=bash
# Thin wrapper: runs the quality gate from the repository root.
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

cd "${ROOT_DIR}"
bash "${ROOT_DIR}/scripts/quality.sh"
