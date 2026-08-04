# Feature Specification: Manual Validation Catalog

**Feature Branch**: `005-manual-validation`

**Created**: 2026-08-04

**Date**: 2026-08-04

**Status**: Draft

**Input**: User description: "Ative a spec 002, mova as tarefas manuais para a 005, renomeando para etapas manuais. Testes automatizados continuam nas specs normais; todo teste manual concentrado na 005 de validações manuais."

**Supersedes**: `005-automated-flow-testing` (policy inverted 2026-08-04 — catalog now owns **manual** validation only).

## Feature Overview

**Feature Name**: Manual Validation Catalog  
**Short Name**: manual-validation  
**Version**: 1.0.0  

Consolidate **manual** validation work for Philarmony into this specification. This feature is the **sole home** for open manual test/validation tasks: hardware smoke, packaging host smoke, quickstart checklist sign-off, and exploratory device/UI validation across product surfaces.

**Manual vs automated split (non-negotiable):**
- **Automated** (Unity/host flow suites, Flutter flow/widget suites, CI test gates) → tracked and implemented on the **product** specs (`001`–`004` and future product features). Automation continues in those specs.
- **Manual** (hardware smoke on real devices, packaging host smoke, quickstart checklist sign-off, exploratory HW/UI validation) → tracked **only** under this catalog (`005-manual-validation`). Tasks SHOULD be named after the manual stage they close (e.g. VS-1, VS-4, VS-5).
- **Historical** completed (`[x]`) automated tasks on product specs stay as history; they are not rewritten or deleted.

Product feature specs keep user journeys, Independent Test descriptions, acceptance criteria, and **automated** test-implementation tasks. They MUST NOT accumulate new open **manual** validation tasks.

## Clarifications

### Session 2026-08-04 — Policy inversion (manual catalog)

- Q: Where do open manual validation tasks live? → A: Only under `specs/005-manual-validation`.
- Q: Where do automated test tasks live? → A: On the product specs that own the journeys (`001`–`004`).
- Q: What about former 002 T031/T062? → A: Moved here and renamed to VS-1 / VS-4 / VS-5 stage tasks.
- Q: What about completed `[x]` automated tasks on 001/002? → A: Remain historical on those product specs.

### Audit results (2026-08-04, post-move)

| Spec | Open automated-test tasks | Open manual test tasks | Notes |
|------|---------------------------|------------------------|-------|
| `001-filament-dryer-esp32` | Historical `[x]` only | None | Automation stays on product spec |
| `002-esp32-desktop-installer` | Historical `[x]` only | None (moved) | T031/T062 → 005 VS-1/VS-4/VS-5 |
| `003-filament-dryer-control-app` | None yet (no `tasks.md`) | None | Future automation on 003; future manual → 005 |
| `004-esp32-touchscreen-ui` | None yet (no `tasks.md`) | None | Future automation on 004; future manual → 005 |
| `005-manual-validation` | N/A | **VS-1, VS-4, VS-5** (from 002) | Sole home for open manual work |

**Verdict**: No open manual tasks remain on product specs. Manual stages for desktop installer live here.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Catalog Owns Manual Stages; Product Specs Keep Automation (Priority: P1)

A contributor or agent can tell, without ambiguity, that new **manual** validation work belongs here (named by stage), while **automated** test work stays on the product feature’s `tasks.md`.

**Why this priority**: Prevents scattering HW/checklist gates and keeps product specs focused on buildable + automatable work.

**Independent Test**: Audit open `[MANUAL]` / smoke / checklist tasks across `specs/*/tasks.md`; any open manual task outside `005-manual-validation` fails the policy. Automated tasks on 001–004 are allowed.

**Acceptance Scenarios**:

1. **Given** a new hardware smoke or checklist sign-off is required, **When** work is planned, **Then** the task is added under `specs/005-manual-validation` with a stage-aligned name (e.g. VS-1).
2. **Given** a new automated flow/widget test is required, **When** work is planned, **Then** the task is added on the owning product spec (`001`–`004`), not here.
3. **Given** a completed `[x]` automated task on 001/002, **When** cleanup runs, **Then** it remains historical and is not deleted.
4. **Given** an agent finds an open manual remnant on a product spec, **When** policy is applied, **Then** that task is moved here (or closed if duplicate) and removed from the product open list.

---

### User Story 2 - Desktop Installer Manual Stages (Priority: P1)

Release confidence for the desktop installer requires completing named manual stages against real hardware and host packaging, recorded in the product quickstart checklist.

**Why this priority**: Automation cannot fully replace USB flash on real ESP32 or host installer smoke.

**Independent Test**: Checklist rows VS-1, VS-4, and VS-5 in `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md` are signed off (PASS) with notes.

**Acceptance Scenarios**:

1. **Given** bundled esptool + synced firmware, **When** VS-1 runs on a real ESP32, **Then** wizard flash succeeds and soft network check never fails the flash.
2. **Given** packaging scripts for Windows/macOS, **When** VS-4 runs on a host, **Then** the produced installer artifact launches and reaches the wizard.
3. **Given** packaging scripts for Linux, **When** VS-5 runs on a Linux host, **Then** the produced AppImage/deb/rpm artifact launches and reaches the wizard.

---

### User Story 3 - Future Product Manual Stages (Priority: P2)

As control-app and touchscreen features gain implementation, their device/UI manual smokes are catalogued here as stage-named tasks before release sign-off.

**Why this priority**: Keeps 003/004 task lists free of open manual work while preserving a single place for HW/UI gates.

**Independent Test**: For each shipped product surface that requires manual sign-off, this catalog lists an open or completed stage task referencing the product checklist path.

**Acceptance Scenarios**:

1. **Given** a control-app device smoke is required, **When** tracked, **Then** the task lives under this catalog (not 003 open list).
2. **Given** a touchscreen UI smoke is required, **When** tracked, **Then** the same catalog rule applies.

---

### Edge Cases

- Product feature adds a journey that needs both automation and HW smoke: automation task on product `tasks.md`; manual stage task here.
- Hardware-unavailable environments: manual stages remain open until hardware is available; they do not block CI automated gates.
- Conflicting guidance: this catalog + constitution win for **manual** task placement; product specs win for journey definition and **automated** test placement.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The project MUST maintain this specification as the single catalog for **manual** validation tasks (hardware smoke, packaging host smoke, quickstart/checklist sign-off).
- **FR-002**: Feature specifications other than this catalog MUST NOT add new open standalone **manual** validation tasks after this policy’s effective date; they MAY keep user journeys, Independent Test descriptions, acceptance criteria, and **automated** test-implementation tasks.
- **FR-003**: Agents and contributors MUST place any new manual validation work discovered during converge/implement/review into this catalog’s tasks rather than appending to product feature open lists.
- **FR-004**: Manual tasks in this catalog MUST be named after the validation stage they close (e.g. VS-1 First flash, VS-4 Host Win/macOS, VS-5 Host Linux) and MUST reference the product checklist path for recording results.
- **FR-005**: Automated flow/CI tests remain the merge gate per constitution; completing manual stages is required for release confidence but does not replace CI automation.
- **FR-006**: Open manual remnants found on product specs MUST be moved here or closed as duplicate.
- **FR-007**: Completed (`[x]`) automated test tasks on product specs MUST be retained as historical record.

### Key Entities

- **Manual Stage Task**: One human-executed validation step (VS-*) mapped to a product checklist row; includes preconditions, actions, expected outcomes, and record path.
- **Product Checklist**: Feature-owned markdown checklist where PASS/PENDING results are written (e.g. 002 quickstart-validation).
- **Coverage Mapping Entry**: Link from a product journey/checklist scenario to a Manual Stage Task id in this feature.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of new **manual** validation tasks created after this policy’s effective date appear under `specs/005-manual-validation`.
- **SC-002**: 100% of open tasks on product specs 001–004 that are smoke/checklist/manual are empty (zero open manual remnants).
- **SC-003**: Desktop installer VS-1, VS-4, and VS-5 each have a distinct open or completed task in this catalog.
- **SC-004**: Contributors can locate the correct checklist path for each open manual stage in under 2 minutes from this specification or `tasks.md`.

## Assumptions

- Product specs 001–004 remain the source of truth for user journeys and automated test implementation.
- Manual stages for 002 reuse `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md` as the results ledger.
- Effective date of the inverted concentration policy is the ratification of constitution v1.0.0.

## Out of Scope

- Redefining product UX, hardware pinouts, or installer packaging formats
- Deleting or rewriting completed historical automated tasks on product specs
- Replacing product feature specs’ User Scenarios & Testing narrative sections
- Moving automated test-implementation tasks into this catalog

**Constitution compliance:** PROJECT_NAME = Philarmony; `.specify/memory/constitution.md` points to this specification as the sole catalog for **manual** validation tasks; automated tests continue on product specs.
