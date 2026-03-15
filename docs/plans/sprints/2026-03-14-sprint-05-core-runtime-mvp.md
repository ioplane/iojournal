# Sprint 05: Core Runtime MVP Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** land the first runnable `iojournal` core with bounded event capture, queueing, filters, redaction, and JSON console output.

**Architecture:** implement the thinnest end-to-end slice that honors the contracts from Sprints 02-04. Do not start file or syslog delivery in this sprint.

**Tech Stack:** C23, CMake, Unity, JSON encoder, console sink.

---

### Task 1: Initialize the implementation tree and build surface

**Files:**
- Create: `CMakeLists.txt`
- Create: `CMakePresets.json`
- Create: `include/iojournal/iojournal.h`
- Create: `src/ij_internal.h`
- Create: `src/core/*.c`
- Create: `tests/unit/CMakeLists.txt`

- [x] **Step 1: Write the failing build check**

Run: `cmake --preset clang-debug`
Expected: fails because the build surface does not exist yet.

- [x] **Step 2: Add the initial build system**

Expected: debug configure and build targets exist.

- [x] **Step 3: Verify configure succeeds**

Run: `cmake --preset clang-debug`
Expected: configure completes successfully.

### Task 2: Implement the bounded event path

**Files:**
- Create: `include/iojournal/iojournal_types.h`
- Create: `src/core/ij_event.c`
- Create: `src/core/ij_queue.c`
- Create: `src/filters/ij_level_filter.c`
- Create: `src/filters/ij_redact.c`
- Create: `src/encoders/ij_json_console.c`
- Create: `src/sinks/ij_console_sink.c`

- [x] **Step 1: Write the failing core tests**

Run: targeted unit tests for event creation, queue push/pop, filtering, and redaction
Expected: fail before implementation exists.

- [x] **Step 2: Implement the minimal core**

Expected: bounded event capture, queue path, filter path, redaction, and JSON console output exist.

- [x] **Step 3: Verify core tests pass**

Run: `ctest --preset clang-debug --output-on-failure`
Expected: core MVP tests pass.

### Task 3: Add example and quality integration

**Files:**
- Create: `examples/basic_console.c`
- Modify: `scripts/quality.sh`

- [x] **Step 1: Add a runnable example**

Expected: example demonstrates console logging through the new core.

- [x] **Step 2: Wire the quality pipeline**

Expected: `scripts/quality.sh` uses real CMake and test commands once the build surface exists.

- [x] **Step 3: Verify the build and example**

Run: `cmake --build --preset clang-debug && ctest --preset clang-debug`
Expected: build and tests pass.
