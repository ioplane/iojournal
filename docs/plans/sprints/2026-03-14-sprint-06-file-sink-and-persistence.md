# Sprint 06: File Sink And Persistence Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** add the RC local persistence story through an NDJSON file sink with rotation and retention baseline.

**Architecture:** extend the Sprint 05 core without widening the feature set beyond file persistence. Keep durability and failure reporting explicit and bounded.

**Tech Stack:** C23, NDJSON formatting, file I/O, Unity tests, examples.

---

### Task 1: Implement NDJSON file output

**Files:**
- Create: `src/encoders/ij_ndjson.c`
- Create: `src/sinks/ij_file_sink.c`
- Create: `include/iojournal/iojournal_file.h`
- Create: `tests/unit/test_file_sink.c`

- [x] **Step 1: Write the failing file sink tests**

Run: targeted file sink tests
Expected: fail before file sink implementation exists.

- [x] **Step 2: Implement the NDJSON file sink**

Expected: append-only NDJSON output works with bounded formatting assumptions.

- [x] **Step 3: Verify file sink tests pass**

Run: `ctest --preset clang-debug --output-on-failure -R file`
Expected: file sink tests pass.

### Task 2: Add rotation, retention, and error reporting baseline

**Files:**
- Modify: `src/sinks/ij_file_sink.c`
- Create: `tests/unit/test_file_rotation.c`
- Create: `docs/en/02-file-sink.md`
- Create: `docs/ru/02-file-sink.md`

- [x] **Step 1: Write the failing rotation tests**

Run: targeted rotation tests
Expected: fail before rotation and retention logic exists.

- [x] **Step 2: Implement minimal rotation/retention**

Expected: size/time rotation baseline and retention bounds are in place.

- [x] **Step 3: Document operational limits**

Expected: file sink docs explain NDJSON output, rotation, retention, and failure semantics.

- [x] **Step 4: Verify docs and tests**

Run: `ctest --preset clang-debug --output-on-failure -R file && python3 scripts/lint-docs.py`
Expected: file sink tests pass and docs lint remains clean.

### Task 3: Add file sink example and release impact notes

**Files:**
- Create: `examples/file_sink.c`
- Modify: `CHANGELOG.md`

- [x] **Step 1: Add example**

Expected: file sink example writes RC-format NDJSON output.

- [x] **Step 2: Update changelog**

Expected: file sink behavior is recorded for the upcoming RC.

- [x] **Step 3: Verify integration**

Run: full quality gate
Expected: repository checks pass with file sink included.
