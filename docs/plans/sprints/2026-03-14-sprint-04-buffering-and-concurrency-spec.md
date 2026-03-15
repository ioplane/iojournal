# Sprint 04: Buffering And Concurrency Spec Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** choose the RC queueing model and freeze ordering, capacity, and overflow behavior before core runtime implementation starts.

**Architecture:** define one primary queue topology for the RC and treat alternatives as deferred. Make memory-ordering and hot-path constraints explicit enough to guide tests and code review.

**Tech Stack:** Markdown specs, concurrency matrices, test strategy docs.

---

### Task 1: Choose the RC queue model

**Files:**
- Create: `docs/architecture/QUEUE_MODEL.md`
- Create: `docs/architecture/BACKPRESSURE_POLICY.md`

- [x] **Step 1: Write the failing queue decision check**

Run: `find docs/architecture -maxdepth 1 -type f | sort`
Expected: queue and backpressure specs do not exist yet.

- [x] **Step 2: Freeze the primary queue model**

Expected: SPSC vs MPSC decision for the RC is explicit, with alternatives deferred.

- [x] **Step 3: Freeze backpressure policy**

Expected: `DROP_NEW`, `DROP_OLD`, blocking exclusions, and metric obligations are documented.

- [x] **Step 4: Verify the decision vocabulary**

Run: `rg -n 'SPSC|MPSC|DROP_NEW|DROP_OLD|backpressure' docs/architecture`
Expected: queue and overflow terms are explicit.

### Task 2: Define memory ordering and layout rules

**Files:**
- Create: `docs/architecture/MEMORY_ORDERING.md`
- Create: `docs/architecture/HOT_PATH_LIMITS.md`

- [x] **Step 1: Define atomic ordering rules**

Expected: acquire/release assumptions and ownership transitions are written down.

- [x] **Step 2: Define hot-path limits**

Expected: no-allocation, bounded-copy, and cacheline/layout constraints are explicit.

- [x] **Step 3: Verify the constraints**

Run: `rg -n 'acquire|release|cache|allocation|bounded' docs/architecture`
Expected: key ordering and hot-path constraints are present.

### Task 3: Build the concurrency test matrix

**Files:**
- Create: `docs/testing/CONCURRENCY_MATRIX.md`
- Create: `docs/testing/STRESS_PLAN.md`

- [x] **Step 1: Define mandatory concurrency cases**

Expected: overflow, full/empty boundaries, reordering risks, producer/consumer races, and redaction-path interactions are listed.

- [x] **Step 2: Define stress strategy**

Expected: sanitizer, loop count, randomized schedule, and evidence format are specified.

- [x] **Step 3: Verify mapping to queue choices**

Run: `rg -n 'overflow|ordering|ABA|producer|consumer' docs/testing`
Expected: the stress plan and matrix cover the chosen queue model.
