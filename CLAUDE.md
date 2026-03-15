# iojournal - Project Instructions for Claude Code

## Quick Facts

- **Language**: C23 (`-std=c23`, `CMAKE_C_EXTENSIONS OFF`)
- **Project**: structured logging library for the `io*` ecosystem
- **License**: intended to remain compatible with the surrounding `io*` stack
- **Platform**: Linux-first development with `io_uring`-aware sink optimization
- **Primary use**: shared logging foundation for `iohttp`, `liboas`, and related components

## Build Commands

Use these commands once the build system is present:

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
cmake --build --preset clang-debug --target format
cmake --build --preset clang-debug --target format-check
```

During bootstrap stages, use `./scripts/quality.sh` for the repository-level checks that can run before the CMake surface exists.

## Dev Container

- Build: `podman build -t iojournal-dev:latest -f deploy/podman/Containerfile .`
- Run: `podman run --rm -it -v $(pwd):/workspace:Z iojournal-dev:latest`
- Prefer doing development and quality checks inside the container.

## Key Directories

```text
include/iojournal/       # Public API headers
src/                     # Core, encoders, sinks, filters, internal runtime
tests/unit/              # Unity tests
examples/                # Small usage examples
docs/plans/              # Architecture and implementation plans
docs/rfc/                # Local RFC mirror
docs/tmp/                # Local scratch area, not source of truth
.claude/skills/          # Repository-local skills and roadmap
```

## Architecture Rules

- Keep the pipeline split explicit: producer -> buffer -> filter -> encoder -> sink.
- Keep the hot logging path bounded and free of dynamic allocation.
- Keep transport, exporter, and sink behavior separate from event capture and filtering.
- Default to safe redaction and conservative backpressure behavior.
- Treat `iohttp`, `liboas`, and other consumers as integration profiles, not as reasons to blur library boundaries.

## Code Conventions

- Public symbols use `ij_`; macros and enum values use `IJ_`; typedefs end with `_t`.
- Use `nullptr`, `[[nodiscard]]`, `_Static_assert`, and checked arithmetic where size math matters.
- Keep pointer style right-aligned: `int *ptr`.
- Match the existing public API style in `include/iojournal/` when that surface lands.

## Testing Rules

- Add or update unit tests for every behavior change once the test harness exists.
- Prefer explicit tests for redaction, overflow policy, timestamp formatting, and sink/encoder boundaries.
- Stress-test queue ownership, ring capacity logic, and concurrency-sensitive code.
- Benchmark only after scalar correctness and safety checks are stable.

## Skills Reference

Base skills live in `.claude/skills/`:
- `iojournal-architecture`
- `iojournal-coding-standards`
- `logging-rfc-reference`
- `modern-c23`
- `iojournal-repository-conventions`

Roadmap: `.claude/skills/ROADMAP.md`
