# Feature Specification: Automated Flow Testing Catalog

**Feature Branch**: `005-automated-flow-testing`

**Created**: 2026-08-04

**Date**: 2026-08-04

**Status**: Draft

**Input**: User description: "Crie um novo spec concentrando apenas as tasks de teste, e crie uma diretriz no projeto que os testes deverão se concentrar nessa especificação específica"

## Feature Overview

**Feature Name**: Automated Flow Testing Catalog  
**Short Name**: automated-flow-testing  
**Version**: 0.1.0  

Consolidate all automated testing work for Philarmony into a single specification. This feature is the **sole home** for test tasks: planning, authoring, expanding, and gating complete user-journey flow tests across firmware, desktop installer, control app, and touchscreen UI. Product feature specs (001–004) keep user journeys and acceptance criteria; they do **not** accumulate new standalone test-implementation tasks.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Single Catalog for All Test Work (Priority: P1)

A contributor or agent looking for “what tests exist / what to add next” opens this specification (and its later plan/tasks) and finds the complete, current inventory of automated flow-test work—without hunting across feature specs.

**Why this priority**: Without a single catalog, test tasks scatter across features, CI gates drift, and constitutional “flow testing only” cannot be audited.

**Independent Test**: Audit that every open automated-test task in the repository is listed under this feature’s `tasks.md` (once generated), and that feature specs 001–004 contain no new unchecked test-implementation tasks after this policy takes effect.

**Acceptance Scenarios**:

1. **Given** a new automated test need is discovered, **When** work is planned, **Then** the task is added under `specs/005-automated-flow-testing` (not under 001–004).
2. **Given** a product feature defines a new user journey, **When** that journey needs automation, **Then** the journey remains documented in the product spec and the automation work is tracked only in this testing spec.
3. **Given** an agent runs `/speckit.tasks` or `/speckit.implement` for testing, **When** the active feature is not this catalog, **Then** the agent redirects test-task creation to this specification.

---

### User Story 2 - Firmware Core Journey Coverage (Priority: P1)

Quality reviewers can verify that every P1 firmware user journey (hotspot setup, WebSocket start/stop/status, sensor-fault safe abort, and stated timing/safety bounds) is covered by failing automated flow tests that exercise real handler paths—not tautological stubs.

**Why this priority**: Firmware safety and connectivity are the product’s trust baseline; incomplete or tautological tests hide regressions.

**Independent Test**: Run the project’s mandated host flow-test command; failing any P1 journey or violated success-bound fails the run.

**Acceptance Scenarios**:

1. **Given** an unconfigured device boot path, **When** the hotspot/Wi‑Fi setup journey is exercised in automation, **Then** the test fails if AP-ready / fail-fast timing or credential-handoff outcomes violate the firmware success criteria.
2. **Given** a configured control path, **When** start/stop/status is exercised, **Then** the test fails if status cadence or command outcomes violate the journey contract.
3. **Given** a sensor-fault injection, **When** the safety journey runs, **Then** the test fails if heater cut-off / safe state is not reached within the success-bound window.
4. **Given** a timing assertion, **When** it only compares hardcoded constants to themselves, **Then** that test is rejected as non-compliant and must be replaced with an instrumented, fail-capable assertion.

---

### User Story 3 - Desktop Installer Journey Coverage (Priority: P1)

Quality reviewers can verify the desktop installer wizard end-to-end: invalid PIN blocking, flash pipeline stages with mocked flasher where hardware is unavailable, soft network-verify warning (non-blocking), profile import/export password redaction, and last-session reload without storing real passwords.

**Why this priority**: Installer is the first-run experience; silent packaging/test gaps ship broken setup flows.

**Independent Test**: Run the installer flow-test suite; each journey above has at least one automated scenario that fails when the behavior regresses.

**Acceptance Scenarios**:

1. **Given** invalid pin mapping input, **When** the wizard advances, **Then** automation asserts the flow is blocked with a clear validation outcome.
2. **Given** a mocked successful flash path, **When** the wizard completes, **Then** automation asserts all flash stages and success presentation.
3. **Given** unreachable network verify, **When** flash otherwise succeeds, **Then** automation asserts a warning-only path (setup still completes).
4. **Given** export/import or last-session reload, **When** credentials are involved, **Then** automation asserts real passwords are never persisted in exported/last-session artifacts.

---

### User Story 4 - Control App & Touchscreen Journey Coverage (Priority: P2)

As control-app and touchscreen features move from draft to implementation, their primary user journeys gain automated flow coverage tracked in this catalog before those features are considered release-ready.

**Why this priority**: Those features are not yet task-complete; reserving catalog slots prevents re-scattering tests later.

**Independent Test**: For each shipped P1 journey in specs 003 and 004, this catalog lists a corresponding automated flow-test task with a Done-When that references the product acceptance scenario.

**Acceptance Scenarios**:

1. **Given** a control-app P1 journey is implemented, **When** release readiness is checked, **Then** this catalog shows a completed flow-test task for that journey (or an explicit, time-boxed waiver recorded here).
2. **Given** a touchscreen P1 journey is implemented, **When** release readiness is checked, **Then** the same catalog rule applies.
3. **Given** only product code lands without catalogued tests, **When** CI/review runs, **Then** the change is blocked until a matching test task exists under this specification.

---

### User Story 5 - CI Gate & Contributor Ergonomics (Priority: P1)

Any contributor can run one documented test entrypoint per product surface and see CI fail the pull request when any mandated flow test fails. Test results remain usable as project memory for agents.

**Why this priority**: Constitutional quality gate is non-negotiable; local/CI parity prevents “green locally, red in CI” drift.

**Independent Test**: Open a PR that breaks a known flow assertion; CI fails. Local documented commands reproduce the same failure class.

**Acceptance Scenarios**:

1. **Given** a failing flow test, **When** CI runs on a pull request, **Then** the build fails and merge is blocked.
2. **Given** documented Make (or equivalent) test targets, **When** a contributor runs them locally, **Then** the same suites CI relies on are exercised.
3. **Given** a green suite run, **When** results are produced, **Then** outcomes are available for catalog/agent reference per project memory rules.

---

### Edge Cases

- Product feature adds a journey but automation is deferred: deferral MUST be an explicit task or waiver in this catalog with rationale—not silence in the product `tasks.md`.
- Hardware-unavailable environments: flow tests MAY use approved simulation/injection seams; they MUST NOT mock away the behavior under assertion (safety cut-off, wizard gate, redaction).
- Legacy test tasks already closed under 001/002 remain historical; **new** test work MUST land here.
- Conflicting guidance between a product plan’s Testing section and this catalog: this catalog + constitution win for task placement; product specs win for journey definition/acceptance intent.
- Tautological or vacuous tests: treated as missing coverage until replaced.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The project MUST maintain this specification as the single catalog for automated testing tasks (creation, prioritization, implementation tracking, and Done-When criteria).
- **FR-002**: Feature specifications other than this catalog MUST NOT add new standalone automated-test implementation tasks after this policy’s effective date; they MAY keep user journeys, Independent Test descriptions, and acceptance criteria.
- **FR-003**: Agents and contributors MUST place any new automated-test work discovered during converge/implement/review into this catalog’s plan/tasks (creating or updating them via Speckit) rather than appending to product feature task lists.
- **FR-004**: Mandated tests MUST represent complete user flows/journeys aligned to product specs 001–004 (and future product specs), not isolated unit-only coverage as the constitutional quality bar.
- **FR-005**: Flow tests MUST be fail-capable against real behavioral outcomes and published success bounds (timing, safety, redaction, wizard gates)—tautological assertions are non-compliant.
- **FR-006**: CI MUST fail the pull request when any mandated flow test fails.
- **FR-007**: Each product surface in scope (firmware core, desktop installer, and later control app / touchscreen when implemented) MUST expose a documented local command that runs its mandated flow suite with non-zero exit on failure.
- **FR-008**: Hardware interaction in tests MUST use simulation/injection only when the seam preserves the asserted safety or user-visible outcome; unapproved full mocks of the behavior under test are forbidden.
- **FR-009**: This catalog MUST reference product-spec journeys by identity (feature id + story/scenario) so coverage mapping stays auditable without duplicating full product requirements.
- **FR-010**: Waivers or temporary gaps in journey coverage MUST be recorded in this catalog with scope, rationale, and closure condition.

### Key Entities

- **Flow Test Case**: One automated scenario mapped to a product user journey/acceptance scenario; includes preconditions, actions, expected outcomes, and fail conditions.
- **Coverage Mapping Entry**: Link from a product journey (spec id + story) to one or more Flow Test Cases and their task ids in this feature.
- **Quality Gate**: CI/local command contract that must pass before merge.
- **Coverage Waiver**: Explicit, time-bounded exception recorded when a journey lacks automation.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of new automated-test tasks created after this policy’s effective date appear under `specs/005-automated-flow-testing` (audit of open tasks across `specs/*/tasks.md`).
- **SC-002**: 100% of firmware P1 journeys listed in this catalog have at least one fail-capable automated flow scenario.
- **SC-003**: 100% of desktop-installer P1 journeys listed in this catalog have at least one fail-capable automated flow scenario.
- **SC-004**: When control-app or touchscreen P1 journeys ship, 100% of those shipped journeys have catalogued automation (or an explicit waiver) before release.
- **SC-005**: 100% of pull requests that break a mandated flow assertion are rejected by CI (no merge with failing mandated suites).
- **SC-006**: Contributors can locate the correct local test command for each in-scope surface in under 2 minutes from project documentation (README / this feature’s quickstart once planned).

## Assumptions

- Existing completed test tasks under specs 001 and 002 remain valid history; this catalog owns **forward** test work and any residual/open testing debt moved here during planning/tasks.
- Product specs 001–004 remain the source of truth for user journeys and acceptance intent; this spec does not redefine product behavior except where testing policy requires fail-capable verification.
- “Flow tests only as the mandatory bar” follows the constitution; additional unit tests may exist but do not satisfy the mandatory gate by themselves.
- Control app (003) and touchscreen (004) are draft/not fully tasked; catalog entries for them may start as planned coverage slots.
- Make/`pio test`/`flutter test` (or successors) remain acceptable local entrypoints once documented; exact tooling is deferred to plan.
- Effective date of the concentration policy is the ratification of the constitution amendment that points to this specification.

## Out of Scope

- Redefining product UX, hardware pinouts, or installer packaging formats
- Manual-only exploratory test charters (unless later explicitly added)
- Performance benchmarking suites unrelated to stated success bounds
- Replacing product feature specs’ User Scenarios & Testing narrative sections

**Constitution compliance:** PROJECT_NAME = Philarmony; testing concentration principle in `.specify/memory/constitution.md` points to this specification as the sole catalog for automated test tasks.
