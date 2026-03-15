[![GitHub](https://img.shields.io/badge/GitHub-iojournal-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iojournal)
[![Issues](https://img.shields.io/badge/GitHub-Issues-181717?style=for-the-badge&logo=github)](https://github.com/ioplane/iojournal/issues)

# Support

## Scope

This repository provides a bounded C23 logging library.

## Supported Channels

- GitHub issues for bugs, build failures, documentation defects, and verification regressions
- pull requests for reproducible fixes and documentation updates

## Required Information For Bug Reports

- commit id
- host and container environment
- exact reproduction sample
- commands used
- expected result
- actual result

## Required Information For Performance Reports

- commit id
- scenario name
- iteration count
- median results
- comparison baseline
- profiler evidence when available

## Unsupported Requests

- application-specific log schema design
- OTLP, HEC, Elastic, or TLS syslog support before the planned sprint
- unbounded queue semantics
- persistence or transport behavior outside the documented RC contract
