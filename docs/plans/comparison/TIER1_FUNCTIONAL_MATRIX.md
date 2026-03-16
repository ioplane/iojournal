# Tier 1 Functional Matrix

## Scope And Tier Freeze

- Tier 1 release-blocking set: `iojournal`, `zlog`, `stumpless`, `tinylog`
- Tier 2 appendix-only set: `microlog`, `rxi/log.c`, `zf_log`, `clog`, `liblogax`, `EasyLogger`
- This matrix is limited to pure C libraries relevant to `iohttp` and `ioguard`.
- Sources of record:
  - [`iojournal`](https://github.com/ioplane/iojournal)
  - [`zlog`](https://github.com/HardySimpson/zlog)
  - [`stumpless`](https://github.com/goatshriek/stumpless)
  - [`tinylog`](https://github.com/pymumu/tinylog)

## Normalized Matrix

| Library | License | Platform And Build Model | Config Model | File Logging And Rotation | Syslog Or Network Logging | Structured Logging | Concurrency Model | Dependency Burden | Consumer Fit Summary | RC Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `iojournal` | Apache-2.0 | C23, CMake presets, Podman-first toolchain | Programmatic API plus frozen RC contracts in repo docs | Native file sink, NDJSON output, size and time rotation, retention, redaction on disk | Native RFC 5424 formatter, RFC 5426 UDP, RFC 6587 TCP | Native event attributes, NDJSON contract, RFC 5424 mapping, redaction policy | Bounded `MPSC -> single consumer` queue, synchronous console and file paths in RC | Repository-local toolchain only | Built for the target consumers; current baseline matches the RC contract pack | Baseline under evaluation |
| `zlog` | Apache-2.0 | Pure C, upstream `make`, repository also carries CMake and Bazel files, POSIX-oriented install path | External config file with categories, formats, and rules | Native files, dynamic file paths, size-limited file rules, process-safe rotation | Native syslog target and user-defined output callbacks | Category and MDC support, but no native JSON or RFC 5424 structured-data contract in the upstream surface | Thread-safe, process-safe rotation, runtime config reload | Low external dependency burden, but requires filesystem config management and POSIX runtime assumptions | Strong file-oriented server logger, but weaker fit for contract-first structured logging and portable sink parity | Partially comparable |
| `stumpless` | Apache-2.0 | Pure C, CMake-first, broad platform support including Linux, Windows, macOS, FreeBSD, DOS | Programmatic target construction, no config-file-first model required | Native file and stream targets, but no built-in rotation or retention contract in the primary feature set | Native Unix socket, TCP, UDP, journald, Windows Event Log, sqlite, and custom targets | Native structured and unstructured logging with RFC 5424 orientation | Thread-safe, compile-time log pruning available | Low dependency burden for core build, optional target-specific surface expands capability set | Best Tier 1 match for syslog and structured delivery, but file operations need external rotation policy | Strong partial comparator |
| `tinylog` | MIT | Pure C core with C++ helpers present in repo, CMake and makefile, Linux or UNIX-oriented | Programmatic API, no external config file required | Native file logging, size rotation, archive compression, multi-file streams | No first-class RFC 5424 or general network sink surface in upstream API | No native structured field model beyond formatted messages and custom callbacks | Asynchronous by default, thread-safe, process-safe, optional nonblocking mode | Requires `pthreads`; compression support is part of the advertised file story | Strong file-path comparator for Linux daemons, weak comparator for structured or syslog-heavy consumers | Partially comparable |

## Decision Notes

### `zlog`

- Strengths:
  - mature file-oriented operational model
  - explicit config reload support
  - process-safe rotation and user-defined outputs
- Weaknesses against the RC target:
  - config-file-first model is higher friction for library embedding
  - no native NDJSON or RFC 5424 structured-data contract
  - upstream install and runtime guidance is POSIX-specific

### `stumpless`

- Strengths:
  - closest Tier 1 alternative for RFC 5424 and network sinks
  - broad target surface and portable build story
  - compile-time pruning aligns with disabled-level cost control
- Weaknesses against the RC target:
  - no built-in rotation or retention contract for file operations
  - broader target surface raises integration and verification scope

### `tinylog`

- Strengths:
  - strong Linux daemon fit for async file logging
  - built-in archive rotation and process-safe write story
  - explicit nonblocking mode
- Weaknesses against the RC target:
  - no first-class syslog or structured delivery contract
  - Linux or UNIX focus reduces portability
  - comparison must normalize async versus synchronous paths carefully

## Release-Blocking Use

- Use this document for functional capability comparison only.
- Use shared-scenario performance results as a separate gate in `PERFORMANCE_RESULTS.md`.
- Read `tinylog` performance rows as file-path pressure only, because its async file-first model is not a protocol-equivalent replacement path for `iohttp` or `ioguard`.
- Read `zlog` and `stumpless` performance rows together with their narrower or different contract surfaces instead of as standalone publish criteria.
- Do not convert missing features into synthetic performance rows.
