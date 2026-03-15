# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| 0.1.x   | Yes       |

## Reporting a Vulnerability

Please report security vulnerabilities via GitHub Security Advisories (preferred)
or by email to the maintainers listed in the repository.

Do NOT create public issues for security vulnerabilities.

## Security Practices

- Static analysis with PVS-Studio, clang-tidy, and CodeChecker
- Containerized verification for reproducible builds
- Mandatory redaction before console, file, and syslog output
- Bounded queue and bounded sink behavior in the RC surface
- No TLS syslog, OTLP, HEC, or Elastic transports in `v0.1.0-rc.1`
