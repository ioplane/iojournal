# Sprint 14: Shell Script Modernization Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rewrite all 14 shell scripts to modern Bash 5.1+ standards with full shellcheck 0.11.0 compliance, structured error handling, cleanup traps, and consistent style. Add shellcheck to the quality gate.

**Architecture:** Extract shared patterns into `scripts/lib/common.sh` (also installed to `/usr/local/lib/ioplane/common.sh` in the base image), then rewrite each script to source it.

**Tech Stack:** Bash 5.1+, shellcheck 0.11.0.

**Status:** COMPLETE. All 7 tasks done. shellcheck clean on 15 files, quality.sh PASS 15/15.

---

### Task 1: Create shared shell library — DONE

File: `scripts/lib/common.sh` (394 lines)
Also installed to ioplane-base image at `/usr/local/lib/ioplane/common.sh`.

Provides: `ioj_pass/fail/skip/info/step`, `ioj_record_pass/fail/skip`, `ioj_print_summary`, `ioj_need_cmd`, `ioj_has_cmake_surface`, `ioj_git_head_short`, `ioj_git_safe_directory`, `ioj_nproc`, `ioj_mktemp`, `ioj_cleanup` trap, `ioj_check_repo_baseline`, `ioj_check_docs_lint`, `ioj_check_build_and_test`, `ioj_check_format`, `ioj_check_cppcheck`, `ioj_check_pvs_studio`, `ioj_check_gcc_analyzer`, `ioj_check_codechecker`, `ioj_check_shellcheck`.

### Task 2: Rewrite quality.sh — DONE

File: `scripts/quality.sh` (rewritten to 12 steps, was 11)
New step 12: shellcheck gate. Sources common.sh. All SC2059/SC2155 fixed.

### Task 3: Rewrite release scripts — DONE

Files: `scripts/run-release-gate.sh`, `scripts/run-release-candidate.sh`, `scripts/build-release-assets.sh`, `scripts/render-release-notes.sh`

### Task 4: Rewrite analysis scripts — DONE

Files: `scripts/run-gcc-analyzer.sh`, `scripts/run-coverage.sh`

### Task 5: Rewrite benchmark scripts — DONE

Files: `scripts/run-benchmarks.sh`, `scripts/run-tier1-benchmarks.sh`, `scripts/build-tier1-competitors.sh`, `scripts/fetch-tier1-competitors.sh`
Fixed: `-j4` → `-j"$(nproc)"`, SC2016 directives for intentional literal `$`.

### Task 6: Rewrite profiler scripts — DONE

Files: `scripts/run-profiler-review.sh`, `scripts/run-podman-perf-lane.sh`, `scripts/build-uftrace-bench.sh`
Fixed: SC2004 (`$` in arithmetic), SC2016 for manifest generation, SC2034 (unused ROOT_DIR).

### Task 7: Add shellcheck to quality gate — DONE

shellcheck step added to `quality.sh` (step 12/12). shellcheck 0.11.0 installed in ioplane-base image.

### Sprint 14 Exit Criteria — MET

- [x] `shellcheck --shell=bash --severity=style scripts/*.sh scripts/lib/*.sh` exits 0
- [x] All scripts source `scripts/lib/common.sh`
- [x] No script uses `echo` for output (all use `printf` or helpers)
- [x] All constants use `readonly`
- [x] All local variable assignments separated from declarations
- [x] All temp files cleaned up via EXIT trap
- [x] quality.sh includes shellcheck as gate step (step 12)
- [x] Quality gate passes in Podman: PASS 15/15
- [x] No behavioral regression

### Unplanned work completed during Sprint 14

- Created `ioplane-base` shared image (3.99 GB) with all common tools
- Rewrote Containerfiles for all 5 io* projects as thin layers
- Updated CLAUDE.md in all 5 projects with base image reference
- Created PRs in iohttpparser (#42), liboas (#5), iohttp (#1), ioguard (#22)
- Removed ringwall-dev (deprecated) containers and images
- Cleaned disk: 32 GB free, 0 dangling images
