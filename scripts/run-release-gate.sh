#!/usr/bin/env bash
# shellcheck shell=bash
# Thin wrapper: runs the quality gate from the repository root.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
readonly ROOT_DIR

cd "${ROOT_DIR}"
python "${ROOT_DIR}/scripts/quality.py"
