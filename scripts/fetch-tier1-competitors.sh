#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_ROOT="${SRC_ROOT:-$ROOT_DIR/docs/tmp/competitors/src}"

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
