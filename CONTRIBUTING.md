# Contributing to iojournal

## Development Environment

All development must happen inside the dev container:

```bash
podman build -t iojournal-dev:latest -f deploy/podman/Containerfile .
podman run --rm -it -v $(pwd):/workspace:Z iojournal-dev:latest
```

## Build & Test

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```

## Code Style

- C23 (`-std=c23`), no extensions
- Linux kernel brace style, 100 column limit
- `clang-format` enforced (see `.clang-format`)
- Pointer style: `int *ptr` (right-aligned)
- Naming: `ij_` prefix, `snake_case` functions, `IJ_` macros/enums, `_t` suffix for types

## Quality Pipeline

```bash
./scripts/quality.sh
```

Runs: build, tests, clang-format, cppcheck, PVS-Studio, CodeChecker.

## Commit Style

Conventional commits: `feat:`, `fix:`, `refactor:`, `test:`, `docs:`.

## Pull Requests

- All tests must pass
- `clang-format` clean
- Zero PVS-Studio errors/warnings
- Zero CodeChecker HIGH/MEDIUM findings
