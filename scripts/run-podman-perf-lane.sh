#!/usr/bin/env bash
# shellcheck shell=bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
if [[ -f /usr/local/lib/ioplane/common.sh ]]; then
    # shellcheck source=/dev/null
    source /usr/local/lib/ioplane/common.sh
else
    # shellcheck source=lib/common.sh disable=SC1091
    source "${SCRIPT_DIR}/lib/common.sh"
fi

ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
readonly ROOT_DIR
readonly PODMAN_BIN="${PODMAN_BIN:-podman}"
readonly IMAGE="localhost/iojournal-dev:latest"
readonly WORKSPACE_HOST="${ROOT_DIR}"
readonly WORKSPACE_CONTAINER="/workspace"

usage() {
    cat <<'EOF'
usage: scripts/run-podman-perf-lane.sh [command...]

Run a command inside the official Podman performance lane for iojournal.

This lane is reserved for:
  - io_uring relevance and implementation experiments
  - uftrace, callgrind, hyperfine, and gdb profiling runs
  - ptrace-sensitive diagnostics that do not work under the default seccomp profile

Environment overrides:
  PODMAN_BIN                  podman executable to use

Examples:
  bash scripts/run-podman-perf-lane.sh bash
  bash scripts/run-podman-perf-lane.sh bash scripts/run-profiler-review.sh auto 3000
EOF
}

if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
    usage
    exit 0
fi

TTY_ARGS=()
if [[ -t 0 && -t 1 ]]; then
    TTY_ARGS=(-it)
fi

PODMAN_ARGS=(
    run
    --rm
    "${TTY_ARGS[@]}"
    --security-opt
    seccomp=unconfined
    --cap-add
    SYS_PTRACE
    --env
    IOJOURNAL_PODMAN_PERF_LANE=1
    -v
    "${WORKSPACE_HOST}:${WORKSPACE_CONTAINER}:Z"
    -w
    "${WORKSPACE_CONTAINER}"
)

COMMAND=(bash)
if [[ "$#" -gt 0 ]]; then
    COMMAND=("$@")
fi

exec "${PODMAN_BIN}" "${PODMAN_ARGS[@]}" "${IMAGE}" "${COMMAND[@]}"
