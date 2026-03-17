# Tier 2 Appendix

## Scope

- Tier 2 libraries provide ecosystem context.
- Tier 2 does not block `v0.1.0-rc.1`.
- Tier 2 entries remain outside the shared-score release gate unless a future roadmap update promotes one into Tier 1.

## Tier 2 Classification

| Library | Why It Is Not Tier 1 | Useful Signal For `iojournal` | Current Classification |
| --- | --- | --- | --- |
| [`microlog`](https://github.com/an-dr/microlog) | Lightweight and configurable, but primarily positioned as a two-file or embedded-friendly logger rather than a server-grade file and syslog competitor | Compile-time stripping, topic filtering, and low-friction packaging | Reference-only |
| [`rxi/log.c`](https://github.com/rxi/log.c) | Too small and too minimal for the target consumer surface; no rotation, no syslog protocol layer, no structured contract | Lower-bound API and footprint baseline | Reference-only |
| [`zf_log`](https://github.com/wonder-mice/zf_log) | Minimal logging core rather than an operational sink framework | Disabled-level cost model and small integration footprint | Reference-only |
| [`clog`](https://github.com/0xA1M/clog) | Header-only and focused on low-level simplicity, not on file rotation, syslog delivery, or structured event contracts | Small-footprint and signal-safe design ideas | Reference-only |
| [`liblogax`](https://github.com/exoticlibraries/liblogax) | Interesting output shaping, but limited ecosystem signal and not a proven server-grade replacement path for the target consumers | JSON and key-value output concepts | Reference-only |
| [`EasyLogger`](https://github.com/armink/EasyLogger) | Embedded and RTOS orientation makes it a poor direct comparator for `iohttp` and `ioguard` deployment patterns | Embedded footprint and portability ideas | Reference-only |

## Tier 2 Use Rules

- Do not use Tier 2 libraries to block or unblock the first public RC.
- Do not mix Tier 2 numbers into the Tier 1 shared comparison table.
- Use Tier 2 only for:
  - appendix narrative
  - future backlog candidates
  - narrow design reference points such as compile-time stripping or minimal API shape

## Deferred Promotion Criteria

A Tier 2 library may be promoted into Tier 1 only if all of the following become true:

- it is a realistic candidate for `iohttp` or `ioguard`
- it exposes enough common surface for normalized functional comparison
- it can be built and benchmarked inside the repository Podman toolchain
- it adds new decision value rather than duplicating an existing Tier 1 comparator
