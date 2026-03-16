# Sprint 10: Functional And Performance Comparison Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** produce the release-blocking Tier 1 comparison pack for `zlog`, `stumpless`, and `tinylog`, with Tier 2 appendix context.

**Architecture:** this sprint compares `iojournal` against plausible pure C alternatives for `iohttp` and `ioguard`. It does not expand the RC feature surface.

**Tech Stack:** Podman, benchmark harness, profiler scripts, comparison matrices, raw artifacts.

**Current status:** document generation, the authoritative `clang-perf` local baseline [`20260316-114500`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-114500), refreshed Tier 1 competitor evidence, `clang-perf` and `clang-uftrace` parity, profiler artifacts [`20260316-114800`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-114800), repeatability framing, and final comparison synthesis are complete.

---

### Task 1: Build the functional comparison matrix

**Files:**
- Create: `docs/plans/comparison/TIER1_FUNCTIONAL_MATRIX.md`
- Create: `docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md`
- Create: `docs/plans/comparison/TIER2_APPENDIX.md`

- [x] **Step 1: Freeze Tier 1 and Tier 2 scope**

Expected: the comparison set is explicit and tied back to the roadmap.

- [x] **Step 2: Produce functional fit analysis**

Expected: buildability, sink model, operability, and license fit are documented for all Tier 1 libraries.

- [x] **Step 3: Verify cross-references**

Run: `grep -RniE 'zlog|stumpless|tinylog|Tier 1|Tier 2' docs/plans/comparison docs/plans`
Expected: comparison docs are internally consistent.

### Task 2: Produce performance comparison evidence

**Files:**
- Create: `docs/plans/comparison/PERFORMANCE_RESULTS.md`
- Create: `docs/plans/comparison/RAW_ARTIFACTS.md`
- Modify: `scripts/run-benchmarks.sh`
- Modify: `scripts/run-profiler-review.sh`

- [x] **Step 1: Capture normalized benchmark data**

Run: `bash scripts/run-benchmarks.sh`
Expected: raw benchmark artifacts exist for the shared scenarios.

- [x] **Step 2: Capture profiler-backed evidence**

Run: `bash scripts/run-profiler-review.sh`
Expected: profiler artifacts and hotspot conclusions exist for `iojournal`.

- [x] **Step 3: Publish normalized performance tables**

Expected: shared scenarios and capability-specific scenarios are separated cleanly.

- [x] **Step 4: Close toolchain and preset parity**

Run: `bash scripts/build-uftrace-bench.sh`
Expected: release-facing `clang-perf` and `clang-uftrace` evidence are both captured and linked from the comparison pack.

### Task 3: Verify the comparison gate

**Files:**
- Modify: `docs/plans/ROADMAP.md`
- Modify: `docs/plans/2026-03-14-iojournal-v0.1.0-rc1-master-plan.md`

- [x] **Step 1: Record comparison completion criteria**

Expected: roadmap and master plan point to the comparison artifacts.

- [x] **Step 2: Verify the evidence pack**

Run: `python3 scripts/lint-docs.py`
Expected: comparison docs are stable and lint-clean.

- [x] **Step 3: Record final consumer-fit conclusion**

Expected: `iohttp` and `ioguard` fit docs incorporate both functional and performance evidence and leave no open Sprint 10 blocker.

### Closure Evidence

- [x] Toolchain parity closed: `diagtool`, `hyperfine`, and `uftrace` are present in the Podman image, and both `clang-perf` and `clang-uftrace` presets were validated.
- [x] The full quality gate passed again after Task 3 with `cppcheck`, `PVS-Studio`, GCC analyzer, and CodeChecker clean.
- [x] The final local performance source of record is [`20260316-114500`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-114500).
- [x] The final profiler source of record is [`20260316-114800`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-114800).
