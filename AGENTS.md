# Repository Guidelines

## Project Structure & Module Organization
`src/` is intended to host the logging implementation split by responsibility: `core` (ring buffers, threads, context), `encoders` (JSON, RFC5424, OTLP payload shaping), `sinks` (console, file, syslog, OTLP, HEC, Elastic), and `filters` (level, sampling, redact). Public headers belong in `include/iojournal/`; internal-only declarations stay in `src/ij_internal.h`. Unit tests belong in `tests/unit/`, integration tests and examples in `examples/`, and development tooling in `scripts/` plus `deploy/podman/`. Long-form plans belong in `docs/plans/`; `docs/tmp/` is scratch space and must not be treated as authoritative.

Bootstrap note: parts of the implementation tree may still be missing while repository infrastructure is being assembled. Treat this document as the intended steady-state layout.

## Build, Test, and Development Commands
Develop inside the container defined in `deploy/podman/Containerfile`:
```bash
podman build -t iojournal-dev:latest -f deploy/podman/Containerfile .
podman run --rm -it -v $(pwd):/workspace:Z iojournal-dev:latest
```

Primary local workflow once the CMake project is present:
```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```

Use `cmake --build --preset clang-debug --target format` to rewrite formatting and `--target format-check` to verify it. Run `./scripts/quality.sh` before submitting changes. During bootstrap stages, the quality script may skip build-specific steps until `CMakeLists.txt` and presets exist.

## Profiling Toolchain
The development image is expected to carry profiling/debug tools for log hot-path work:
- `gdb`
- `valgrind`
- `uftrace`
- `ftracer` helpers (`frun`, `fresolve`, `/usr/local/lib/ftracer/ftracer.o`)

Use them after a normal debug build. Typical sequence:
```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
```
Then choose the tool that matches the question:
```bash
uftrace record -- ./build/clang-debug/bench/bench_hot_path
valgrind --tool=callgrind ./build/clang-debug/bench/bench_hot_path
gdb --args ./build/clang-debug/bench/bench_hot_path
```

## Required Host Utilities
Keep these available on the host for effective agent and contributor workflows:
- `git`
- `gh` with working `gh api graphql` authentication
- `rg` (`ripgrep`)
- `jq`
- `python3`
- `podman`

Strongly recommended:
- `uv` / `uvx`
- `clangd`
- `fd`
- `yq`
- `hyperfine`
- `gdb`
- `valgrind`
- `uftrace`
- `ftracer`

## Documentation Style
Write English documentation first. Treat `docs/en/*` as authoritative and `docs/ru/*` as the translation/adaptation layer once the stable docs surface exists.

Documentation must use strict technical language:
- write facts, contracts, limits, ownership rules, and examples
- do not write narrative introductions, philosophy sections, or marketing text
- keep comparisons measurable and testable
- keep non-goals explicit and tied to API or layer boundaries
- use Mermaid for diagrams
- use extended GitHub Markdown only where it improves technical readability:
  tables, fenced code blocks with language tags, short blockquote alerts for strict warnings, and `<details>` for secondary examples
- use badges in every stable document; badge links must point to official sources, official repositories, or official project sites
- Russian documentation must use Russian prose; keep English only for API identifiers, macro names, external project names, protocol names, and RFC identifiers

Mermaid diagram type must match the content:
- architecture: `architecture-beta` or `flowchart`
- interaction or handoff: `sequenceDiagram`
- state transitions: `stateDiagram-v2`
- release gates or requirements: `requirementDiagram`
- classification or topology: `mindmap`, `block`, or `flowchart`
- comparison positioning: `quadrantChart` only when axes are explicit and measurable
- prefer the official Mermaid syntax page for the selected diagram type when linking Mermaid badges

Keep the stable structure aligned with other `io*` projects:
- `docs/README.md`
- `docs/en/README.md`
- `docs/ru/README.md`
- numbered documents in `docs/en/` and `docs/ru/`
- `docs/plans/README.md`, `docs/plans/ROADMAP.md`, `docs/plans/BACKLOG.md`

`docs/tmp/` is non-authoritative scratch space.

## Changelog And Versioning
Maintain [CHANGELOG.md](CHANGELOG.md) for every merge that changes behavior, public API, verification evidence, release automation, or published documentation.

Rules:
- Follow [Keep a Changelog 1.1.0](https://keepachangelog.com/en/1.1.0/).
- Keep `Unreleased` at the top.
- Add entries only under the standard sections that apply:
  `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, `Security`.
- Write short factual bullets. Do not write narrative paragraphs in the changelog.
- Record externally visible changes only. Do not list routine refactors unless they change behavior, contract, or release evidence.
- Update the changelog in the same branch as the code or documentation change. Do not defer it to a later cleanup branch.

Versioning rules:
- Use Semantic Versioning tags with a leading `v`.
- Current pre-1.0 policy:
  - `v0.y.z` for normal unstable releases
  - `v0.y.z-rc.N` for release candidates
- Do not create `1.0.0` until public API, release gate, and published evidence are all declared stable.
- For this repository, the first public release candidate line is `v0.1.0-rc.N`.

## Coding Style & Naming Conventions
Target C23 only; C extensions are disabled. Follow `.clang-format`: 4-space indentation, Linux brace style, 100-column limit, and right-aligned pointer stars (`int *ptr`). Keep public and internal symbols consistent with the existing scheme: `ij_` for functions, `IJ_` for macros and enum values, and `_t` for typedefs. Prefer small, single-purpose translation units and keep the hot logging path bounded and allocation-free.

## Testing Guidelines
Unit tests use Unity and are enabled through `IOJOURNAL_BUILD_TESTS`. Name new unit tests `tests/unit/test_<area>.c` and register them in `CMakeLists.txt`. Concurrency structures such as ring buffers and queues require stress coverage for races, ABA risks, and backpressure behavior. Validate at least `ctest --preset clang-debug`; sanitizer presets such as `clang-asan` and `clang-tsan` are preferred for concurrency or memory-safety changes.

## Commit & Pull Request Guidelines
Follow the established convention: `feat:`, `fix:`, `refactor:`, `test:`, and `docs:` with short imperative subjects. Pull requests should describe the behavior change, list the commands run, and note any analyzer results or skipped checks. Include sample inputs or output snippets when modifying sink behavior, structured formats, or redaction rules.
