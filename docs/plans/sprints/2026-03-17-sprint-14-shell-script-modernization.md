# Sprint 14: Shell Script Modernization Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rewrite all 14 shell scripts to modern Bash 5.1+ standards with full shellcheck 0.11.0 compliance, structured error handling, cleanup traps, and consistent style. Add shellcheck to the quality gate.

**Architecture:** Extract shared patterns into `scripts/lib/common.sh`, then rewrite each script group. Every rewritten script must: (1) pass `shellcheck --shell=bash --severity=style`, (2) use `readonly` for constants, (3) use `printf` instead of `echo` for output, (4) have cleanup traps for temp files, (5) declare and assign variables separately (`local var; var=$(...)`), (6) use `[[ ]]` for conditionals, (7) use `main()` function structure for non-trivial scripts.

**Tech Stack:** Bash 5.1+, shellcheck 0.11.0, shfmt (optional).

---

## Shell Modernization Standards

Every rewritten script must comply with these rules:

### Header
```bash
#!/usr/bin/env bash
# shellcheck shell=bash
set -euo pipefail
```

### Constants
```bash
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
```

### Error handling
```bash
cleanup() {
    rm -f "${TEMP_FILE:-}"
}
trap cleanup EXIT
```

### Variable declaration (SC2155)
```bash
# Bad: local var=$(cmd) — masks exit status
# Good:
local var
var=$(cmd)
```

### Output
```bash
# Bad: echo with ANSI escapes
# Good:
printf '%sPASS%s: %s\n' "${GREEN}" "${NC}" "$1"
```

### Quoting (SC2086)
All variable expansions must be double-quoted: `"${var}"`, `"${array[@]}"`.

### Conditionals
Use `[[ ]]` not `[ ]`. Use `command -v` not `which`.

### SC2016 false positives
For TSV generators that intentionally write literal `$` signs, use:
```bash
# shellcheck disable=SC2016
printf '%s\n' 'columns	$benchmark	$scenario'
```

---

### Task 1: Create shared shell library

**Files:**
- Create: `scripts/lib/common.sh`

- [ ] **Step 1: Write the shared library**

```bash
#!/usr/bin/env bash
# shellcheck shell=bash
# Shared utilities for iojournal scripts.
# Source this file: source "${SCRIPT_DIR}/lib/common.sh"

readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[0;33m'
readonly CYAN='\033[0;36m'
readonly NC='\033[0m'

# Print colored status messages
log_pass() { printf '%b%s%b: %s\n' "${GREEN}" "PASS" "${NC}" "$1"; }
log_fail() { printf '%b%s%b: %s\n' "${RED}" "FAIL" "${NC}" "$1" >&2; }
log_skip() { printf '%b%s%b: %s\n' "${YELLOW}" "SKIP" "${NC}" "$1"; }
log_info() { printf '%b%s%b: %s\n' "${CYAN}" "INFO" "${NC}" "$1"; }
log_step() { printf '\n%b=== [%d/%d] %s ===%b\n' "${CYAN}" "$1" "$2" "$3" "${NC}"; }

# Check command availability
need_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        log_fail "missing required command: $1"
        return 1
    fi
}

# Resolve ROOT_DIR from SCRIPT_DIR
resolve_root_dir() {
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[1]}")" && pwd)"
    cd "${script_dir}/.." && pwd
}

# Safe git HEAD short hash
git_head_short() {
    local root_dir="${1:-.}"
    if git -C "${root_dir}" rev-parse --short HEAD 2>/dev/null; then
        return 0
    fi
    printf 'nogit\n'
}

# Ensure git safe directory for container use
git_safe_directory() {
    local root_dir="${1:-.}"
    git config --global --add safe.directory "${root_dir}" 2>/dev/null || true
}

# Check if CMake surface exists
has_cmake_surface() {
    [[ -f "${1:-.}/CMakeLists.txt" && -f "${1:-.}/CMakePresets.json" ]]
}
```

- [ ] **Step 2: Verify with shellcheck**

Run: `shellcheck --shell=bash --severity=style scripts/lib/common.sh`
Expected: exit 0, no warnings.

- [ ] **Step 3: Commit**

---

### Task 2: Rewrite quality.sh

**Files:**
- Modify: `scripts/quality.sh`

This is the largest script (296 lines). Key issues to fix:
- SC2059: printf format string variables (`"${CYAN}..."` used as format string)
- No cleanup trap for temp files (`/tmp/iojournal-*`)
- No `main()` function structure
- Inconsistent quoting

- [ ] **Step 1: Rewrite with modern patterns**

Changes:
1. Source `scripts/lib/common.sh` for colors and helpers
2. Replace `step()`, `ok()`, `fail()`, `skip()` with `log_step`, `log_pass`, `log_fail`, `log_skip`
3. Fix SC2059: use `%b` format specifier for ANSI escapes or pass colors as arguments
4. Add cleanup trap for `/tmp/iojournal-*` temp files
5. Wrap logic in `main()` function
6. Declare and assign local variables separately (SC2155)
7. Use `readonly` for `BUILD_DIR`, `PRESET`, `ROOT_DIR`, `THIRD_PARTY_DIR`

- [ ] **Step 2: Run shellcheck**

Run: `shellcheck --shell=bash --severity=style scripts/quality.sh`
Expected: exit 0.

- [ ] **Step 3: Run quality gate in Podman to verify no behavioral regression**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: PASS: 14, FAIL: 0, SKIP: 0.

- [ ] **Step 4: Commit**

---

### Task 3: Rewrite release scripts

**Files:**
- Modify: `scripts/run-release-gate.sh`
- Modify: `scripts/run-release-candidate.sh`
- Modify: `scripts/build-release-assets.sh`
- Modify: `scripts/render-release-notes.sh`

- [ ] **Step 1: Rewrite run-release-gate.sh**

Tiny script (8 lines). Add shellcheck directive, source common.sh, use `readonly`.

- [ ] **Step 2: Rewrite run-release-candidate.sh**

Changes:
1. Source common.sh
2. Use `git_head_short` and `git_safe_directory` helpers
3. Declare and assign separately (SC2155)
4. Use `readonly` for `RUN_ID`, `OUT_BASE`, `OUT_DIR`
5. Add cleanup trap

- [ ] **Step 3: Rewrite build-release-assets.sh**

Changes:
1. Source common.sh
2. Add usage function
3. Use `readonly` for constants
4. `git_safe_directory` helper
5. Clean up subshell for sha256sum

- [ ] **Step 4: Rewrite render-release-notes.sh**

Changes:
1. Source common.sh
2. Declare and assign separately
3. Use `readonly` for constants

- [ ] **Step 5: Run shellcheck on all four**

Run: `shellcheck --shell=bash --severity=style scripts/run-release-gate.sh scripts/run-release-candidate.sh scripts/build-release-assets.sh scripts/render-release-notes.sh`
Expected: exit 0.

- [ ] **Step 6: Run release gate to verify no behavioral regression**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/run-release-candidate.sh`
Expected: RC run completes successfully.

- [ ] **Step 7: Commit**

---

### Task 4: Rewrite analysis scripts

**Files:**
- Modify: `scripts/run-gcc-analyzer.sh`
- Modify: `scripts/run-coverage.sh`

- [ ] **Step 1: Rewrite run-gcc-analyzer.sh**

Changes:
1. Source common.sh
2. Use `readonly` for `PRESET`, `BUILD_DIR`, `REPORT_DIR`
3. Declare and assign separately
4. Add `main()` structure
5. Use `need_cmd gcc`

- [ ] **Step 2: Rewrite run-coverage.sh**

Changes:
1. Source common.sh
2. Use `readonly` where appropriate
3. Declare and assign separately (SC2155) — especially `mapfile` and local variable assignments
4. Add cleanup trap for profile data
5. Use `main()` structure

- [ ] **Step 3: Run shellcheck**

Run: `shellcheck --shell=bash --severity=style scripts/run-gcc-analyzer.sh scripts/run-coverage.sh`
Expected: exit 0.

- [ ] **Step 4: Verify gcc analyzer runs**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/run-gcc-analyzer.sh`
Expected: lane passes.

- [ ] **Step 5: Commit**

---

### Task 5: Rewrite benchmark scripts

**Files:**
- Modify: `scripts/run-benchmarks.sh`
- Modify: `scripts/run-tier1-benchmarks.sh`
- Modify: `scripts/build-tier1-competitors.sh`
- Modify: `scripts/fetch-tier1-competitors.sh`

- [ ] **Step 1: Rewrite fetch-tier1-competitors.sh**

Changes:
1. Source common.sh
2. Use `readonly` for `SRC_ROOT`
3. Use `need_cmd git`

- [ ] **Step 2: Rewrite build-tier1-competitors.sh**

Changes:
1. Source common.sh
2. Use `readonly` for constants
3. SC2016 false positives: add `# shellcheck disable=SC2016` where single-quoted CMake code is intentional

- [ ] **Step 3: Rewrite run-benchmarks.sh**

Changes:
1. Source common.sh
2. Use `readonly` for immutable vars
3. SC2016: add disable directives for intentional TSV literal `$` expressions
4. Declare and assign separately
5. Add `main()` function

- [ ] **Step 4: Rewrite run-tier1-benchmarks.sh**

Changes:
1. Source common.sh
2. Use `readonly` for constants
3. Declare and assign separately
4. Add `main()` function

- [ ] **Step 5: Run shellcheck on all four**

Run: `shellcheck --shell=bash --severity=style scripts/fetch-tier1-competitors.sh scripts/build-tier1-competitors.sh scripts/run-benchmarks.sh scripts/run-tier1-benchmarks.sh`
Expected: exit 0.

- [ ] **Step 6: Commit**

---

### Task 6: Rewrite profiler scripts

**Files:**
- Modify: `scripts/run-profiler-review.sh`
- Modify: `scripts/run-podman-perf-lane.sh`
- Modify: `scripts/build-uftrace-bench.sh`

- [ ] **Step 1: Rewrite build-uftrace-bench.sh**

Tiny script (8 lines). Fix SC2034 (unused `ROOT_DIR`). Source common.sh. Use `readonly`.

- [ ] **Step 2: Rewrite run-podman-perf-lane.sh**

Changes:
1. Source common.sh
2. Use `readonly` for constants
3. Simplify `TTY_ARGS` array construction

- [ ] **Step 3: Rewrite run-profiler-review.sh**

Largest script (314 lines). Changes:
1. Source common.sh
2. Use `readonly` for immutable vars
3. SC2016: add disable directives for summary manifest printf calls using literal `$`
4. Declare and assign separately throughout
5. Add `main()` function for the case dispatch
6. Use `need_cmd` helper consistently

- [ ] **Step 4: Run shellcheck on all three**

Run: `shellcheck --shell=bash --severity=style scripts/run-profiler-review.sh scripts/run-podman-perf-lane.sh scripts/build-uftrace-bench.sh`
Expected: exit 0.

- [ ] **Step 5: Commit**

---

### Task 7: Add shellcheck to quality gate

**Files:**
- Modify: `scripts/quality.sh`
- Modify: `deploy/podman/Containerfile`

- [ ] **Step 1: Add shellcheck installation to Containerfile**

Add `shellcheck` installation to the dev container build. The binary is available at `https://github.com/koalaman/shellcheck/releases/tag/v0.11.0` or via package manager.

```dockerfile
# Shellcheck for shell script linting
RUN curl -fsSL "https://github.com/koalaman/shellcheck/releases/download/v0.11.0/shellcheck-v0.11.0.linux.x86_64.tar.xz" \
    | tar -xJ --strip-components=1 -C /usr/local/bin shellcheck-v0.11.0/shellcheck
```

- [ ] **Step 2: Add shellcheck step to quality.sh**

Add as a new step (renumber existing steps if needed, or add as step 12):

```bash
log_step N TOTAL "Shellcheck"
if command -v shellcheck >/dev/null 2>&1; then
    if shellcheck --shell=bash --severity=style scripts/*.sh scripts/lib/*.sh; then
        log_pass "Shellcheck clean"
    else
        log_fail "Shellcheck found issues"
    fi
else
    log_skip "shellcheck not installed"
fi
```

- [ ] **Step 3: Build updated container**

Run: `podman build -t iojournal-dev:latest -f deploy/podman/Containerfile .`

- [ ] **Step 4: Run quality gate with shellcheck step**

Run: `podman run --rm --env-file /opt/projects/repositories/iohttpparser/.env -v $(pwd):/workspace:Z -w /workspace localhost/iojournal-dev:latest bash scripts/quality.sh`
Expected: new shellcheck step is PASS, total PASS count increases.

- [ ] **Step 5: Commit**

---

### Sprint 14 Exit Criteria

- `shellcheck --shell=bash --severity=style scripts/*.sh scripts/lib/*.sh` exits 0
- All scripts source `scripts/lib/common.sh`
- No script uses `echo` for output (use `printf` or helpers)
- All constants use `readonly`
- All local variable assignments are separated from declarations
- All temp files are cleaned up via EXIT trap
- `quality.sh` includes shellcheck as a gate step
- Quality gate passes in Podman with all steps green
- No behavioral regression in any script
