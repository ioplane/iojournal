#!/usr/bin/env bash
# shellcheck shell=bash
# Clones or updates tier-1 competitor repositories for benchmarking.
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

readonly SRC_ROOT="${SRC_ROOT:-${ROOT_DIR}/docs/tmp/competitors/src}"

ensure_repo() {
    local name="$1"
    local url="$2"
    local repo_dir="${SRC_ROOT}/${name}"

    mkdir -p "${SRC_ROOT}"
    if [[ ! -d "${repo_dir}/.git" ]]; then
        git clone --depth 1 "${url}" "${repo_dir}"
        return
    fi

    git -C "${repo_dir}" pull --ff-only
}

ensure_repo "zlog" "https://github.com/HardySimpson/zlog.git"
ensure_repo "stumpless" "https://github.com/goatshriek/stumpless.git"
ensure_repo "tinylog" "https://github.com/pymumu/tinylog.git"

printf '%s\n' "${SRC_ROOT}"
