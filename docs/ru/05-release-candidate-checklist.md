[![Semantic Versioning](https://img.shields.io/badge/SemVer-2.0.0-3da639)](https://semver.org/)
[![Keep a Changelog](https://img.shields.io/badge/Keep%20a%20Changelog-1.1.0-8a2be2)](https://keepachangelog.com/en/1.1.0/)
[![GitHub Actions](https://img.shields.io/badge/GitHub%20Actions-release%20gate-181717)](https://docs.github.com/en/actions)

# Checklist Кандидата На Релиз

## Область

- Этот документ задает минимальный publication gate для `v0.1.0-rc.1`.
- Каждое решение о релизе должно ссылаться на конкретные локальные или опубликованные артефакты.
- Отсутствующий owner, отсутствующая remote-проверка или отсутствующий артефакт являются release blocker.

```mermaid
requirementDiagram
    requirement local_quality {
        id: RC-LOCAL
        text: Local quality gate must pass
        risk: medium
        verifymethod: test
    }
    requirement comparison_gate {
        id: RC-COMPARE
        text: Functional and performance comparison evidence must be current
        risk: medium
        verifymethod: analysis
    }
    requirement optimization_gate {
        id: RC-OPT
        text: Optimization evidence must be complete
        risk: medium
        verifymethod: analysis
    }
    requirement governance_gate {
        id: RC-GOV
        text: CODEOWNERS and remote workflow state must be valid
        risk: high
        verifymethod: inspection
    }
```

## Цель

| Поле | Значение |
| --- | --- |
| Линия релиза | `v0.1.0-rc.1` |
| Тип релиза | первый публичный release candidate |
| Текущий локальный benchmark source | [`20260316-210134`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-210134) |
| Текущий локальный profiler source | [`20260316-210204`](/opt/projects/repositories/iojournal/docs/tmp/profiling/20260316-210204) |
| Текущий Tier 1 comparison source | [`20260316-150320`](/opt/projects/repositories/iojournal/docs/tmp/benchmarks/20260316-150320) |

## Publication Gates

| Gate | Требование | Доказательство | Правило статуса |
| --- | --- | --- | --- |
| Local quality | `scripts/quality.sh` проходит внутри Podman development image | `dist/release-candidate/runs/<RUN_ID>/release-gate.txt` и локальный вывод команды | `PASS` только если прогон зеленый на финальном состоянии workspace |
| Stable docs | существуют нумерованные English и Russian документы для publication gate и decision | этот документ и `06-rc1-publish-decision.md` в обеих локалях | `PASS` только если `python3 scripts/lint-docs.py` зеленый |
| Functional comparison | пакет Sprint 10 comparison остается полным | [`PERFORMANCE_RESULTS.md`](/opt/projects/repositories/iojournal/docs/plans/comparison/PERFORMANCE_RESULTS.md), [`TIER1_FUNCTIONAL_MATRIX.md`](/opt/projects/repositories/iojournal/docs/plans/comparison/TIER1_FUNCTIONAL_MATRIX.md), [`IOHTTP_IOGUARD_FIT_MATRIX.md`](/opt/projects/repositories/iojournal/docs/plans/comparison/IOHTTP_IOGUARD_FIT_MATRIX.md) | `PASS` только если Tier 1 evidence актуален и внутренне согласован |
| Optimization evidence | доказательная база Sprint 11A и Sprint 11B завершена и согласована | [`IO_URING_RELEVANCE.md`](/opt/projects/repositories/iojournal/docs/testing/IO_URING_RELEVANCE.md), [`SIMD_RELEVANCE.md`](/opt/projects/repositories/iojournal/docs/testing/SIMD_RELEVANCE.md), [`PERFORMANCE_RESULTS.md`](/opt/projects/repositories/iojournal/docs/plans/comparison/PERFORMANCE_RESULTS.md) | `PASS` только если активные source-of-record benchmark и profiler артефакты названы явно |
| Release candidate run | `scripts/run-release-candidate.sh` пересобирает финальный локальный evidence pack | `dist/release-candidate/runs/<RUN_ID>/summary.md` и `latest.txt` | `PASS` только если прогон сделан по финальному состоянию workspace |
| Release assets | собраны source archive, verification archive, checksums и release notes; zip и docs archive остаются optional | содержимое `dist/` для целевого тега | `PASS` только если существуют все обязательные артефакты |
| Repository ownership | `.github/CODEOWNERS` указывает на реального GitHub user или team | `.github/CODEOWNERS` | `PASS` только если placeholder text удален |
| Remote gate | существует состояние GitHub workflow для финальной pushed revision | GitHub Actions runs на финальном pushed commit | `PASS` только если целевая revision запушена и ее обязательные workflow зеленые |

## Правило Выхода

- Публикация допустима только если каждый gate выше имеет статус `PASS`.
- Решение нужно зафиксировать в `06-rc1-publish-decision.md`.
- Если хотя бы один gate не имеет `PASS`, решение должно быть `NO-PUBLISH`.
