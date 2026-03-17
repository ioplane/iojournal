<p align="center">
  <a href="https://github.com/ioplane/iojournal"><img alt="GitHub" src="https://img.shields.io/badge/GitHub-iojournal-181717?style=for-the-badge&logo=github"></a>
  <a href="https://github.com/ioplane/iojournal/actions/workflows/release.yml"><img alt="Release" src="https://img.shields.io/github/actions/workflow/status/ioplane/iojournal/release.yml?style=for-the-badge&label=Release"></a>
  <a href="https://github.com/ioplane/iojournal/actions/workflows/ci.yml"><img alt="Release Gate" src="https://img.shields.io/github/actions/workflow/status/ioplane/iojournal/ci.yml?branch=main&style=for-the-badge&label=Release%20Gate"></a>
  <a href="https://github.com/ioplane/iojournal/actions/workflows/coverage.yml"><img alt="Coverage" src="https://img.shields.io/github/actions/workflow/status/ioplane/iojournal/coverage.yml?branch=main&style=for-the-badge&label=Coverage"></a>
  <a href="https://codecov.io/github/ioplane/iojournal"><img alt="Codecov" src="https://img.shields.io/codecov/c/github/ioplane/iojournal?style=for-the-badge&logo=codecov"></a>
  <a href="https://scorecard.dev/viewer/?uri=github.com/ioplane/iojournal"><img alt="OpenSSF Scorecard" src="https://api.scorecard.dev/projects/github.com/ioplane/iojournal/badge?style=for-the-badge"></a>
  <a href="https://github.com/ioplane/iojournal/actions/workflows/codeql.yml"><img alt="CodeQL" src="https://img.shields.io/github/actions/workflow/status/ioplane/iojournal/codeql.yml?branch=main&style=for-the-badge&label=CodeQL"></a>
  <a href="https://github.com/ioplane/iojournal/actions/workflows/trivy.yml"><img alt="Trivy" src="https://img.shields.io/github/actions/workflow/status/ioplane/iojournal/trivy.yml?branch=main&style=for-the-badge&label=Trivy"></a>
  <a href="https://sonarcloud.io/project/overview?id=ioplane_iojournal"><img alt="SonarQube Cloud" src="https://img.shields.io/sonar/quality_gate/ioplane_iojournal?server=https%3A%2F%2Fsonarcloud.io&style=for-the-badge"></a>
</p>

<p align="center">
  <a href="https://www.iso.org/standard/82075.html"><img alt="C23" src="https://img.shields.io/badge/ISO-IEC%209899%3A2024-00599C?style=for-the-badge"></a>
  <a href="https://www.rfc-editor.org/rfc/rfc5424.html"><img alt="RFC 5424" src="https://img.shields.io/badge/RFC-5424-1a73e8?style=for-the-badge"></a>
  <a href="https://www.rfc-editor.org/rfc/rfc5426.html"><img alt="RFC 5426" src="https://img.shields.io/badge/RFC-5426-1a73e8?style=for-the-badge"></a>
  <a href="https://www.rfc-editor.org/rfc/rfc6587.html"><img alt="RFC 6587" src="https://img.shields.io/badge/RFC-6587-1a73e8?style=for-the-badge"></a>
  <a href="https://www.doxygen.nl/"><img alt="Doxygen" src="https://img.shields.io/badge/Doxygen-Reference-2C4AA8?style=for-the-badge"></a>
  <a href="docs/en/README.md"><img alt="en" src="https://img.shields.io/badge/lang-en-blue.svg"></a>
  <a href="docs/ru/README.md"><img alt="ru" src="https://img.shields.io/badge/lang-ru-green.svg"></a>
</p>

# iojournal

Bounded logging library for C23.

Release model:
- source-first
- no mandatory prebuilt binaries
- release assets include source archives, generated API reference, verification artifacts, and checksums

## Scope

Included:
- bounded event submission path
- JSON console output
- NDJSON file output with rotation and retention baseline
- RFC 5424 syslog formatting
- RFC 5426 UDP syslog delivery
- RFC 6587 TCP octet-counting syslog delivery
- mandatory redaction on the RC sink surface

Excluded:
- TLS syslog
- OTLP
- Splunk HEC
- Elastic Bulk API
- shared-memory transports
- spool persistence

## Properties

| Property | Value |
|---|---|
| API model | synchronous logger API |
| Queue model | bounded MPSC to single consumer contract |
| Stable sinks | console, file, syslog |
| Syslog modes | RFC 5424 + RFC 5426/RFC 6587 |
| Redaction | enabled by configuration, applied before encoding |
| License | MIT |

## Layer Model

| Layer | Responsibility |
|---|---|
| Core | event validation, copy, queueing, logger lifecycle |
| Filters | level filtering and redaction |
| Encoders | console JSON, NDJSON, RFC 5424 |
| Sinks | console, file, syslog UDP, syslog TCP |

## Minimal Example

```c
#include <iojournal/iojournal.h>

int main(void)
{
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "example.start",
        .message = "hello from iojournal",
        .logger = "readme.example",
    };
    ij_logger_config_t config = {
        .min_level = IJ_LEVEL_INFO,
        .sink_kind = IJ_SINK_KIND_CONSOLE,
        .redaction_mode = IJ_REDACTION_MODE_ENABLED,
        .queue_capacity = IJ_QUEUE_CAPACITY_DEFAULT,
        .logger_name = "readme.example",
    };
    ij_logger_t *logger = NULL;

    if (ij_logger_init(&logger, &config) != IJ_STATUS_OK) {
        return 1;
    }
    if (ij_logger_log(logger, &event) != IJ_STATUS_OK) {
        (void)ij_logger_shutdown(logger);
        return 2;
    }

    return ij_logger_shutdown(logger) == IJ_STATUS_OK ? 0 : 3;
}
```

## Build

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
ctest --preset clang-debug
```

Container workflow:

```bash
podman build -t iojournal-dev:latest -f deploy/podman/Containerfile .
podman run --rm -it -v $(pwd):/workspace:Z iojournal-dev:latest
```

## Public API

| Area | API |
|---|---|
| Logger lifecycle | `ij_logger_init()`, `ij_logger_log()`, `ij_logger_flush()`, `ij_logger_shutdown()` |
| Event model | `ij_event_t`, `ij_attr_t`, `ij_timestamp_t` |
| Sink selection | `ij_sink_kind_t`, `ij_syslog_transport_t`, `ij_syslog_facility_t` |
| Versioning | `ij_version()`, `ij_version_num()` |

## Functional Summary

The current published feature set includes:
- bounded event validation and deep-copy path
- JSON console output
- NDJSON file sink with rotation and retention baseline
- RFC 5424 formatting with facility and severity mapping
- RFC 5426 UDP delivery and RFC 6587 TCP octet-counting delivery
- stable English and Russian docs for bootstrap, file sink, and syslog RC surfaces

Published references:
- [docs/en/01-bootstrap-and-rfc-corpus.md](docs/en/01-bootstrap-and-rfc-corpus.md)
- [docs/en/02-file-sink.md](docs/en/02-file-sink.md)
- [docs/en/03-syslog-contract.md](docs/en/03-syslog-contract.md)
- [docs/en/05-release-candidate-checklist.md](docs/en/05-release-candidate-checklist.md)
- [docs/en/06-rc1-publish-decision.md](docs/en/06-rc1-publish-decision.md)

## References

| Document | Purpose |
|---|---|
| [docs/README.md](docs/README.md) | documentation index |
| [docs/api/README.md](docs/api/README.md) | RC API contract workspace |
| [docs/architecture/QUEUE_MODEL.md](docs/architecture/QUEUE_MODEL.md) | queue topology |
| [docs/testing/CONCURRENCY_MATRIX.md](docs/testing/CONCURRENCY_MATRIX.md) | concurrency validation surface |
| [docs/plans/ROADMAP.md](docs/plans/ROADMAP.md) | roadmap to `v0.1.0-rc.1` |
| [SUPPORT.md](SUPPORT.md) | support channels and required report inputs |

## Status Codes

| Code | Meaning |
|---|---|
| `IJ_STATUS_OK` | operation completed |
| `IJ_STATUS_INVALID_ARGUMENT` | invalid input or configuration |
| `IJ_STATUS_INVALID_STATE` | logger or sink state does not allow the operation |
| `IJ_STATUS_QUEUE_FULL` | bounded queue rejected the event |
| `IJ_STATUS_ENCODE_ERROR` | encoding failed |
| `IJ_STATUS_SINK_ERROR` | sink delivery failed |
| `IJ_STATUS_INTERNAL_ERROR` | internal invariant or allocation failure |

## License

[MIT](LICENSE)
