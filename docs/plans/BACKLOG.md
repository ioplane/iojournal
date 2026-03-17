# iojournal Backlog After v0.1.0-rc.1

Items in this document are deferred by scope. They are not required to ship `v0.1.0-rc.1`.

The standards-side defer rationale is frozen in `docs/rfc/DIVERGENCES_AND_DEFERRALS.md`.

## Protocol And Delivery

| Item | Why deferred from RC | Earliest unlock point |
| --- | --- | --- |
| RFC 5425 syslog over TLS | requires transport security policy, certificate handling, and operational evidence not included in Sprint 07; see `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` | after RC, once syslog UDP/TCP evidence is stable |
| OTLP/HTTP exporter | requires additional schema and transport contracts outside the RC sink set; see `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` | after RC, after API and field mapping stabilize |
| Splunk HEC exporter | vendor-specific delivery contract not covered by the RC protocol family; see `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` | after RC, after exporter surface is introduced |
| Elastic Bulk exporter | vendor-specific bulk semantics not covered by the RC protocol family; see `docs/rfc/DIVERGENCES_AND_DEFERRALS.md` | after RC, after exporter surface is introduced |
| richer ECS and OpenTelemetry field presets | needs broader naming and compatibility policy than the RC minimum | after RC, after base event schema adoption evidence |

## Runtime And Reliability

| Item | Why deferred from RC | Earliest unlock point |
| --- | --- | --- |
| disk-backed spool queues | requires persistence recovery and queue durability contracts beyond Sprint 06 | after RC, after file sink behavior is stable |
| shared-memory multi-process ring transport | requires a second runtime topology and cross-process ownership rules | after RC, after single-process queue model is proven |
| more than one queue topology profile | the RC freezes one queue model in Sprint 04 | after RC, after baseline stress evidence |
| explicit crash-safe recovery rules for persistence | requires failure-mode testing beyond append-only RC persistence | after RC, after persistence API review |
| encryption at rest for archived files | requires key management and operational policy outside RC scope | after RC, after persistence surface expands |

## Performance And Tooling

| Item | Why deferred from RC | Earliest unlock point |
| --- | --- | --- |
| broad `io_uring` backend expansion beyond the Sprint 11A decision scope | the RC optimization sprint evaluates only the minimal proven surface; broader backend rollout still widens runtime scope materially | after RC, after the first optimized RC baseline is published |
| universal SIMD abstraction layer and internal SIMD helper library | the RC line only needs narrow measured kernels and scalar-safe ABI rules; a broader cross-ISA abstraction layer would widen maintenance and verification scope materially | after RC, after the optimized baseline and arm64 evidence are stable |
| `AVX-512`, `SVE`, and `SVE2` exploration | the RC line freezes SIMD to the measured `AVX2` and `NEON` scope; wider ISA work needs dedicated measurement, dispatch policy, and operability evidence | after RC, after the first optimized RC baseline is published |
| zero-copy send state machine for network sinks | depends on proven network sink contracts first | after RC, after syslog interoperability evidence |
| CodeChecker hardening for maximal C23 analysis coverage | requires a separate tooling study and evidence pass to tune analyzer/checker selection, CTU, `clangsa`/`clang-tidy` arguments, sensitive checks, and Z3-backed refutation against the stabilized codebase; see the official CodeChecker docs | after RC, once the base build and sink surface are stable enough to tighten the gate without blocking core feature delivery |
| long-term performance trend archive | depends on a stable benchmark suite and multiple release points | after RC, after the first comparison program is complete |
| expanded competitor matrix beyond Tier 1/Tier 2 | useful after the first RC, but not required for the first publish decision | after RC, after the initial comparison pack is published |
| deep PGO/LTO optimization campaign | requires a stable published baseline before aggressive optimizer tuning work | after RC, after the initial perf review |

## Documentation And Adoption

| Item | Why deferred from RC | Earliest unlock point |
| --- | --- | --- |
| full stable docs surface under `docs/en` and `docs/ru` | Sprint 01 initializes the surface, but the full set depends on later contract and implementation work | continues through and after RC |
| Doxygen API publication | depends on the public header surface existing first | after RC, once headers are stable enough to publish |
| integration guides for `iohttp` and `liboas` | depends on the RC API and sink contracts being fixed | after RC |
| migration notes for post-RC API changes | only meaningful after the first public RC exists | after RC |
| broader standards harvesting beyond the RC-target RFC set | the RC freezes a minimal standards family in Sprint 02 | after RC, when new protocol families are intentionally added |
