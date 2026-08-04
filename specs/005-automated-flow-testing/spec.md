# Feature Specification: Automated Flow Testing Catalog

**Feature Branch**: `005-automated-flow-testing`

**Created**: 2026-08-04

**Date**: 2026-08-04

**Status**: Draft

**Input**: User description: "Crie um novo spec concentrando apenas as tasks de teste, e crie uma diretriz no projeto que os testes deverão se concentrar nessa especificação específica"

**Follow-up input (2026-08-04)**: "Revise todas as specs previamente criadas e verifique se sobrou algum resquício dos testes, exceto em tarefas já executadas. Devemos nos limitar às tarefas de teste manual; os testes automatizados devem continuar sendo implementados."

## Feature Overview

**Feature Name**: Automated Flow Testing Catalog  
**Short Name**: automated-flow-testing  
**Version**: 0.2.0  

Consolidate **automated** testing work for Philarmony into this specification. This feature is the **sole home** for automated test-implementation tasks: planning, authoring, expanding, and gating complete user-journey flow tests across firmware, desktop installer, control app, and touchscreen UI.

**Manual vs automated split (non-negotiable):**
- **Automated** (Unity/host flow suites, Flutter flow/widget suites, CI test gates, fail-capable journey automation) → tracked and implemented via this catalog (`005`). Automation **continues to be implemented**; it is not paused or demoted.
- **Manual** (hardware smoke on real devices, quickstart checklist sign-off, exploratory HW validation) → MAY remain as open tasks on product specs (`001`–`004`). Product specs MUST NOT hold open automated-test implementation tasks.
- **Historical** completed (`[x]`) automated tasks on product specs stay as history; they are not rewritten or deleted.

Product feature specs keep user journeys, Independent Test descriptions, and acceptance criteria. They do **not** accumulate new standalone automated-test implementation tasks.

## Clarifications

### Session 2026-08-04 — Product-spec audit & manual/automated boundary

- Q: Where may open test tasks live? → A: Open **manual** validation tasks on product specs; open **automated** implementation tasks only under `005`.
- Q: What about already-completed automated tasks on 001/002? → A: Keep as historical `[x]` records; do not migrate closed history.
- Q: Does concentrating tests in 005 stop automated implementation? → A: No — automated flow tests continue to be planned and implemented under this catalog.

### Audit results (2026-08-04)

| Spec | Open automated-test tasks | Open manual test tasks | Notes |
|------|---------------------------|------------------------|-------|
| `001-filament-dryer-esp32` | None | None | All automated test tasks `[x]`; Independent Test / quickstart narratives retained |
| `002-esp32-desktop-installer` | None | **T031**, **T062** (HW smoke / quickstart VS-1/4/5) | Allowed manual remnants; annotated `[MANUAL]` |
| `003-filament-dryer-control-app` | None (no `tasks.md`) | None | Journeys only; future automation → `005` |
| `004-esp32-touchscreen-ui` | None (no `tasks.md`) | None | Journeys only; future automation → `005` |
| `005-automated-flow-testing` | (this catalog) | N/A | Sole home for new automated work |

**Verdict**: No incomplete automated-test remnants remain on product specs. Only open test-related work outside `005` is manual (002).

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Catalog Owns Automated Work; Product Specs Keep Manual Only (Priority: P1)

A contributor or agent can tell, without ambiguity, that new automated test work belongs here, while remaining open product-spec test work is limited to manual validation (smoke/checklist). Completed automated history on product specs is left untouched.

**Why this priority**: Prevents re-scattering of automation tasks and preserves legitimate HW/manual gates on product features.

**Independent Test**: Re-run the audit table above; any new unchecked automated-test task under 001–004 fails the policy. Open manual tasks on product specs are allowed.

**Acceptance Scenarios**:

1. **Given** a new automated test need is discovered, **When** work is planned, **Then** the task is added under `specs/005-automated-flow-testing` (not under 001–004).
2. **Given** a hardware smoke or quickstart sign-off is required, **When** the task is tracked, **Then** it MAY remain on the product feature’s `tasks.md` and MUST be marked as manual.
3. **Given** a completed `[x]` automated test task on 001/002, **When** cleanup runs, **Then** it remains historical and is not deleted.
4. **Given** an agent finds an open automated-test remnant on a product spec, **When** policy is applied, **Then** that task is moved to this catalog (or closed if duplicate) and removed from the product open list.

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

Quality reviewers can verify the desktop installer wizard end-to-end via **automation** (invalid PIN blocking, flash stages with mocked flasher, soft network-verify warning, password redaction). Real-device smoke remains a **manual** product-spec task (002 T031/T062) and does not replace automated journey coverage in this catalog.

**Why this priority**: Installer is the first-run experience; silent packaging/test gaps ship broken setup flows.

**Independent Test**: Run the installer flow-test suite; each automated journey above has at least one scenario that fails when the behavior regresses. Manual HW smoke stays tracked on 002 until signed off.

**Acceptance Scenarios**:

1. **Given** invalid pin mapping input, **When** the wizard advances, **Then** automation asserts the flow is blocked with a clear validation outcome.
2. **Given** a mocked successful flash path, **When** the wizard completes, **Then** automation asserts all flash stages and success presentation.
3. **Given** unreachable network verify, **When** flash otherwise succeeds, **Then** automation asserts a warning-only path (setup still completes).
4. **Given** export/import or last-session reload, **When** credentials are involved, **Then** automation asserts real passwords are never persisted in exported/last-session artifacts.
5. **Given** T031/T062 remain open on 002, **When** policy is audited, **Then** they are classified as manual and do not violate the automated-catalog rule.

---

### User Story 4 - Control App & Touchscreen Journey Coverage (Priority: P2)

As control-app and touchscreen features move from draft to implementation, their primary user journeys gain automated flow coverage tracked in this catalog before those features are considered release-ready. Any manual device/UI smoke for those features may live on 003/004 task lists.

**Why this priority**: Those features are not yet task-complete; reserving catalog slots prevents re-scattering automation later.

**Independent Test**: For each shipped P1 journey in specs 003 and 004, this catalog lists a corresponding automated flow-test task with a Done-When that references the product acceptance scenario.

**Acceptance Scenarios**:

1. **Given** a control-app P1 journey is implemented, **When** release readiness is checked, **Then** this catalog shows a completed automated flow-test task for that journey (or an explicit waiver recorded here).
2. **Given** a touchscreen P1 journey is implemented, **When** release readiness is checked, **Then** the same catalog rule applies.
3. **Given** only product code lands without catalogued automation, **When** CI/review runs, **Then** the change is blocked until a matching automated test task exists under this specification.

---

### User Story 5 - CI Gate & Contributor Ergonomics (Priority: P1)

Any contributor can run one documented test entrypoint per product surface and see CI fail the pull request when any mandated **automated** flow test fails. Test results remain usable as project memory for agents.

**Why this priority**: Constitutional quality gate is non-negotiable; local/CI parity prevents “green locally, red in CI” drift.

**Independent Test**: Open a PR that breaks a known flow assertion; CI fails. Local documented commands reproduce the same failure class.

**Acceptance Scenarios**:

1. **Given** a failing automated flow test, **When** CI runs on a pull request, **Then** the build fails and merge is blocked.
2. **Given** documented Make (or equivalent) test targets, **When** a contributor runs them locally, **Then** the same suites CI relies on are exercised.
3. **Given** a green suite run, **When** results are produced, **Then** outcomes are available for catalog/agent reference per project memory rules.

---

### Edge Cases

- Product feature adds a journey but automation is deferred: deferral MUST be an explicit task or waiver in this catalog—not an open automated task on the product `tasks.md`.
- Manual HW smoke may stay on the product spec even when automation for the same journey exists here; manual does not substitute for the automated gate.
- Hardware-unavailable environments: automated flow tests MAY use approved simulation/injection seams; they MUST NOT mock away the behavior under assertion.
- Legacy completed automated tasks under 001/002 remain historical; **new open** automated work MUST land here.
- Conflicting guidance between a product plan’s Testing section and this catalog: this catalog + constitution win for **automated** task placement; product specs win for journey definition and **manual** smoke placement.
- Tautological or vacuous automated tests: treated as missing coverage until replaced.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The project MUST maintain this specification as the single catalog for **automated** testing tasks (creation, prioritization, implementation tracking, and Done-When criteria). Automated tests MUST continue to be implemented under this catalog.
- **FR-002**: Feature specifications other than this catalog MUST NOT add new open standalone **automated**-test implementation tasks after this policy’s effective date; they MAY keep user journeys, Independent Test descriptions, acceptance criteria, and **manual** validation tasks (hardware smoke, quickstart checklist sign-off).
- **FR-003**: Agents and contributors MUST place any new automated-test work discovered during converge/implement/review into this catalog’s plan/tasks rather than appending to product feature task lists.
- **FR-004**: Mandated automated tests MUST represent complete user flows/journeys aligned to product specs 001–004 (and future product specs), not isolated unit-only coverage as the constitutional quality bar.
- **FR-005**: Automated flow tests MUST be fail-capable against real behavioral outcomes and published success bounds—tautological assertions are non-compliant.
- **FR-006**: CI MUST fail the pull request when any mandated automated flow test fails.
- **FR-007**: Each product surface in scope MUST expose a documented local command that runs its mandated automated flow suite with non-zero exit on failure.
- **FR-008**: Hardware interaction in automated tests MUST use simulation/injection only when the seam preserves the asserted safety or user-visible outcome; unapproved full mocks of the behavior under test are forbidden.
- **FR-009**: This catalog MUST reference product-spec journeys by identity (feature id + story/scenario) so coverage mapping stays auditable without duplicating full product requirements.
- **FR-010**: Waivers or temporary gaps in automated journey coverage MUST be recorded in this catalog with scope, rationale, and closure condition.
- **FR-011**: Open test-related tasks remaining on product specs MUST be limited to **manual** validation; any open automated remnant found there MUST be moved to this catalog or closed as duplicate.
- **FR-012**: Completed (`[x]`) automated test tasks on product specs MUST be retained as historical record unless an explicit migration rewrite is approved.

### Key Entities

- **Flow Test Case**: One automated scenario mapped to a product user journey/acceptance scenario; includes preconditions, actions, expected outcomes, and fail conditions.
- **Manual Validation Task**: Hardware smoke or checklist sign-off tracked on a product feature; not a substitute for automated gates.
- **Coverage Mapping Entry**: Link from a product journey (spec id + story) to one or more Flow Test Cases and their task ids in this feature.
- **Quality Gate**: CI/local automated command contract that must pass before merge.
- **Coverage Waiver**: Explicit, time-bounded exception recorded when a journey lacks automation.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of new **automated**-test tasks created after this policy’s effective date appear under `specs/005-automated-flow-testing` (audit of open automated tasks across `specs/*/tasks.md`).
- **SC-002**: 100% of open test-related tasks on product specs 001–004 are classified as manual (or the set is empty).
- **SC-003**: 100% of firmware P1 journeys listed in this catalog have at least one fail-capable automated flow scenario.
- **SC-004**: 100% of desktop-installer P1 journeys listed in this catalog have at least one fail-capable automated flow scenario (independent of manual T031/T062 completion).
- **SC-005**: When control-app or touchscreen P1 journeys ship, 100% of those shipped journeys have catalogued automation (or an explicit waiver) before release.
- **SC-006**: 100% of pull requests that break a mandated automated flow assertion are rejected by CI.
- **SC-007**: Contributors can locate the correct local automated test command for each in-scope surface in under 2 minutes from project documentation.

## Assumptions

- Existing completed automated test tasks under specs 001 and 002 remain valid history; this catalog owns **forward automated** work.
- Product specs 001–004 remain the source of truth for user journeys and acceptance intent.
- “Flow tests only as the mandatory automated bar” follows the constitution; additional unit tests may exist but do not satisfy the mandatory gate by themselves.
- Manual HW smoke (e.g. 002 T031/T062) is necessary for release confidence and is intentionally outside this catalog’s task list.
- Control app (003) and touchscreen (004) are draft/not fully tasked; catalog entries for them may start as planned coverage slots.
- Make/`pio test`/`flutter test` (or successors) remain acceptable local entrypoints once documented; exact tooling is deferred to plan.
- Effective date of the concentration policy is the ratification of the constitution amendment that points to this specification.

## Out of Scope

- Redefining product UX, hardware pinouts, or installer packaging formats
- Deleting or rewriting completed historical automated tasks on product specs
- Replacing product feature specs’ User Scenarios & Testing narrative sections
- Moving manual HW smoke / quickstart sign-off tasks into this catalog (they stay on product specs)
- Performance benchmarking suites unrelated to stated success bounds

**Constitution compliance:** PROJECT_NAME = Philarmony; `.specify/memory/constitution.md` points to this specification as the sole catalog for **automated** test tasks; product specs may retain **manual** validation tasks only.
