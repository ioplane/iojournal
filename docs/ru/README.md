# iojournal Russian Documentation

## Назначение

Этот каталог содержит стабильный набор русскоязычной документации `iojournal`.

## Стабильные документы

- `01-bootstrap-and-rfc-corpus.md`
  стартовый контракт Sprint 01 для сбора RFC, локального зеркала RFC и инициализации стабильной документации.
- `02-file-sink.md`
  контракт Sprint 06 для файлового приемника RC: NDJSON-персистентность, rotation, retention и обработка ошибок.
- `03-syslog-contract.md`
  контракт Sprint 07 для syslog RC: форматирование RFC 5424, доставка UDP по RFC 5426 и framing TCP по RFC 6587.
- `04-tooling-and-agent-workflow.md`
  стабильный workflow-контракт для Podman, инструментов Clang 22, profiling/debug stack и ролей skills/agents.
- `05-release-candidate-checklist.md`
  стабильный checklist публикации `v0.1.0-rc.1`, включая local quality, comparison, optimization и repository-governance gates.
- `06-rc1-publish-decision.md`
  явная фиксация решения publish или no-publish для `v0.1.0-rc.1`.

## Текущая навигация

- Сначала читайте `01-bootstrap-and-rfc-corpus.md`, если меняете `docs/rfc/*` или артефакты Sprint 01.
- Сначала читайте `02-file-sink.md`, если меняете RC file sink path, тесты персистентности или правила retention.
- Сначала читайте `03-syslog-contract.md`, если меняете форматирование RFC 5424, transport-поведение syslog или артефакты Sprint 07.
- Сначала читайте `04-tooling-and-agent-workflow.md`, если меняете контейнерный workflow, стек Clang, profiling stack, skills или agent policy integration.
- Сначала читайте `05-release-candidate-checklist.md`, если меняете критерии публикации RC, входы release-gate или требования Sprint 12 к доказательной базе.
- Сначала читайте `06-rc1-publish-decision.md`, если собираетесь утверждать, что репозиторий готов к публичной публикации `v0.1.0-rc.1`.
- Используйте `docs/api/README.md` для текущего набора контрактов Sprint 03 по интерфейсу первого кандидата на релиз; это техническая рабочая поверхность, а не стабильный нумерованный документ.
- Используйте `docs/rfc/README.md` для команд обслуживания корпуса RFC и границ ответственности по артефактам.
- Используйте `docs/plans/ROADMAP.md` для контрольных точек и зависимостей.

## Следующие документы

- архитектура и границы проекта
- API и модель данных
- материалы по кандидату на релиз
- post-RC hardening notes

## Связанные каталоги

- `docs/plans/`
- `docs/rfc/`
