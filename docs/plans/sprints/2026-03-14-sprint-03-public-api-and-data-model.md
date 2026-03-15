# Sprint 03: Public API And Data Model Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** freeze the first RC public API surface, event schema, ownership model, and error model.

**Architecture:** treat this sprint as the contract freeze for all later code. The output is a tight API pack and examples, not implementation logic.

**Tech Stack:** Markdown API docs, header planning, example snippets, test list docs.

---

### Task 1: Define the RC public API surface

**Files:**
- Create: `docs/api/README.md`
- Create: `docs/api/RC_API_SURFACE.md`
- Create: `docs/api/RC_ERROR_MODEL.md`

- [x] **Step 1: Write the failing surface inventory**

Run: `find docs/api -maxdepth 1 -type f | sort`
Expected: no API contract pack yet.

- [x] **Step 2: Define the API surface**

Expected: public types, init/config entry points, logging calls, and sink-facing boundaries are named.

- [x] **Step 3: Define status and error model**

Expected: success/failure model and error categories are explicit.

- [x] **Step 4: Verify API terms**

Run: `rg -n 'ij_|error|status|config|sink|event' docs/api`
Expected: the main public contract vocabulary is represented.

### Task 2: Define the event and attribute model

**Files:**
- Create: `docs/api/EVENT_MODEL.md`
- Create: `docs/api/ATTRIBUTE_MODEL.md`
- Create: `docs/api/OWNERSHIP_AND_LIFETIME.md`

- [x] **Step 1: Freeze event shape**

Expected: `ij_event_t` fields, severity, timestamps, message/body, and metadata are specified.

- [x] **Step 2: Freeze attribute representation**

Expected: key/value constraints, limits, string ownership, and trace field conventions are specified.

- [x] **Step 3: Freeze ownership and lifetime rules**

Expected: producer-owned memory vs copied memory rules are explicit.

- [x] **Step 4: Verify consistency**

Run: `rg -n 'ownership|lifetime|attribute|timestamp|trace|redaction' docs/api`
Expected: cross-document consistency around event and attribute semantics.

### Task 3: Define API examples and test obligations

**Files:**
- Create: `docs/api/EXAMPLES.md`
- Create: `docs/api/TEST_OBLIGATIONS.md`

- [x] **Step 1: Add positive examples**

Expected: basic console/file/syslog usage sketches exist.

- [x] **Step 2: Add negative and boundary test obligations**

Expected: redaction, overflow, invalid fields, and lifetime violations are listed as mandatory test areas.

- [x] **Step 3: Verify coverage of RC features**

Run: `rg -n 'console|file|syslog|redaction|overflow' docs/api/EXAMPLES.md docs/api/TEST_OBLIGATIONS.md`
Expected: RC scope is fully represented.
