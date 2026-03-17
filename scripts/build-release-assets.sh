#!/usr/bin/env bash
# shellcheck shell=bash
# Builds source tarballs, optional docs/verification archives, and checksums.
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

readonly DIST_DIR="${ROOT_DIR}/dist"
readonly TAG_NAME="${1:-${GITHUB_REF_NAME:-dev}}"
readonly ARCHIVE_PREFIX="iojournal-${TAG_NAME}"

mkdir -p "${DIST_DIR}"
rm -f \
    "${DIST_DIR}/${ARCHIVE_PREFIX}.tar.gz" \
    "${DIST_DIR}/${ARCHIVE_PREFIX}.zip" \
    "${DIST_DIR}/${ARCHIVE_PREFIX}-docs.tar.gz" \
    "${DIST_DIR}/${ARCHIVE_PREFIX}-verification.tar.gz" \
    "${DIST_DIR}/${ARCHIVE_PREFIX}.sha256" \
    "${DIST_DIR}/RELEASE_NOTES.md"

if git -C "${ROOT_DIR}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    ioj_git_safe_directory "${ROOT_DIR}"
    git -C "${ROOT_DIR}" archive \
        --format=tar.gz \
        --prefix="${ARCHIVE_PREFIX}/" \
        -o "${DIST_DIR}/${ARCHIVE_PREFIX}.tar.gz" \
        HEAD

    if command -v zip >/dev/null 2>&1; then
        git -C "${ROOT_DIR}" archive \
            --format=zip \
            --prefix="${ARCHIVE_PREFIX}/" \
            -o "${DIST_DIR}/${ARCHIVE_PREFIX}.zip" \
            HEAD
    fi
else
    tar --exclude="${DIST_DIR}" --exclude="${ROOT_DIR}/build" \
        -C "${ROOT_DIR}" -czf "${DIST_DIR}/${ARCHIVE_PREFIX}.tar.gz" .
    if command -v zip >/dev/null 2>&1; then
        (
            cd "${ROOT_DIR}"
            zip -qr "${DIST_DIR}/${ARCHIVE_PREFIX}.zip" . -x "dist/*" "build/*"
        )
    fi
fi

if [[ -d "${ROOT_DIR}/docs/api/html" ]]; then
    tar -C "${ROOT_DIR}/docs/api/html" \
        -czf "${DIST_DIR}/${ARCHIVE_PREFIX}-docs.tar.gz" .
fi

if [[ -d "${ROOT_DIR}/dist/release-candidate" ]]; then
    tar -C "${ROOT_DIR}/dist" \
        -czf "${DIST_DIR}/${ARCHIVE_PREFIX}-verification.tar.gz" release-candidate
fi

bash "${ROOT_DIR}/scripts/render-release-notes.sh" "${TAG_NAME}"

(
    cd "${DIST_DIR}"
    : > "${ARCHIVE_PREFIX}.sha256"
    for file in \
        "${ARCHIVE_PREFIX}.tar.gz" \
        "${ARCHIVE_PREFIX}.zip" \
        "${ARCHIVE_PREFIX}-docs.tar.gz" \
        "${ARCHIVE_PREFIX}-verification.tar.gz"
    do
        [[ -f "${file}" ]] || continue
        sha256sum "${file}" >> "${ARCHIVE_PREFIX}.sha256"
    done
)
