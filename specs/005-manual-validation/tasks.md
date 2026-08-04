# Tasks: Manual Validation Catalog

**Input**: Design documents from `/specs/005-manual-validation/`  
**Prerequisites**: `spec.md` (policy inversion 2026-08-04); product checklists under owning features  
**Active product context**: Desktop installer stages migrated from `002` T031/T062  

**Tests policy**: This catalog tracks **manual only**. Automated flow/widget/CI tasks stay on product specs (`001`–`004`).

## Format: `- [ ] [ID] [P?] [Story?] Description with file path`

- **[P]**: Parallelizable across different hosts/devices  
- **[Story]**: [US1]…[US3]  
- Stage names (VS-*) MUST appear in the task title

## Path Conventions

- Catalog: `specs/005-manual-validation/`
- Installer checklist ledger: `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md`

---

## Phase 1: Setup (Catalog scaffolding)

**Purpose**: Ensure the manual catalog is the only open home for smoke/checklist work

- [x] T001 Rename catalog dir to `specs/005-manual-validation/` and point policy docs at manual-only scope
- [x] T002 [P] Activate product feature `002` in `.specify/feature.json`
- [x] T003 [P] Close migrated open manuals on `specs/002-esp32-desktop-installer/tasks.md` with pointers here

**Checkpoint**: No open `[MANUAL]` tasks remain under `specs/001`–`004`

---

## Phase 2: Foundational (Recording conventions)

**Purpose**: Shared rules for signing off stages

- [x] T004 Document that results are written to the owning product checklist (not duplicated here) in `specs/005-manual-validation/spec.md`
- [x] T005 [P] Keep `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md` as the VS-* results ledger for installer stages

**Checkpoint**: Each open stage task below names its ledger path

---

## Phase 3: User Story 1 — Policy ownership (Priority: P1) 🎯

**Goal**: Manual-only catalog; automation remains on product specs

**Independent Test**: Audit open manual tasks across specs — only this file has open manuals

- [x] T006 [US1] Encode inverted placement rule in `.specify/memory/constitution.md` (v1.0.0)
- [x] T007 [P] [US1] Update product-spec policy banners on `specs/001`–`004` `spec.md` files
- [x] T008 [P] [US1] Update `.specify/templates/tasks-template.md` Tests guidance for manual→005 / automation→product

**Checkpoint**: Agents redirect new manual work here and new automation to product specs

---

## Phase 4: User Story 2 — Desktop installer manual stages (Priority: P1)

**Goal**: Close VS-1 / VS-4 / VS-5 for the desktop installer on real hardware and host packages

**Independent Test**: Rows VS-1, VS-4, VS-5 in the 002 quickstart checklist are PASS with notes

### Migrated from 002 (renamed to stage names)

- [ ] T009 [US2] [MANUAL] **VS-1 First-flash hardware smoke** — run `make bundle-esptool` + `make sync-installer-firmware` + `make run`, flash real ESP32 through Device→…→Install; soft network check must not fail flash; record model/port/host OS in `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md` (was 002 T031)
- [ ] T010 [P] [US2] [MANUAL] **VS-4 Host packaging smoke (Windows/macOS)** — build host installer via packaging scripts, launch artifact, reach wizard; record result in `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md` (was part of 002 T062)
- [ ] T011 [P] [US2] [MANUAL] **VS-5 Host packaging smoke (Linux)** — build AppImage/deb/rpm, launch artifact, reach wizard; record result in `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md` (was part of 002 T062)

**Checkpoint**: Installer manual stages signed off; 002 product implementation tasks remain complete

---

## Phase 5: User Story 3 — Future product manual stages (Priority: P2)

**Goal**: Placeholder lane for 003/004 device/UI smokes when those features need release gates

**Independent Test**: When 003/004 require manual smoke, tasks are appended here (not on product open lists)

- [ ] T012 [US3] [MANUAL] **VS-CTRL-1 Control-app device smoke** — when 003 ships P1 journeys, add/execute device discovery + dashboard smoke and record under `specs/003-filament-dryer-control-app/checklists/` (create checklist if missing)
- [ ] T013 [US3] [MANUAL] **VS-UI-1 Touchscreen UI smoke** — when 004 ships P1 journeys, add/execute on-device UI smoke and record under `specs/004-esp32-touchscreen-ui/checklists/` (create checklist if missing)

**Checkpoint**: No open manual tasks appear on 003/004 `tasks.md`

---

## Phase 6: Polish & Cross-Cutting

- [x] T014 Sync README roadmap / 005 section for manual-validation catalog in `README.md`
- [ ] T015 [MANUAL] **VS-AUDIT Catalog audit** — re-scan `specs/*/tasks.md` for open manual remnants outside this file; fix or move if found

---

## Dependencies & Execution Order

```text
Done: T001–T008, T014 (policy + migration scaffolding)
Open manuals (human): T009 (VS-1) then T010 // T011 (VS-4 // VS-5)
Future: T012–T013 when 003/004 need smoke
Polish: T015 audit after stage closures
```

### Parallel Opportunities

- T010 // T011 (different host OS families)
- T012 // T013 (different product surfaces, when ready)

---

## Implementation Strategy

### MVP close-out (installer release confidence)

1. T009 — VS-1 real ESP32 flash smoke  
2. T010 + T011 — VS-4 / VS-5 host packaging smoke  
3. T015 — confirm zero open manuals on product specs  

### Suggested MVP Scope

**US2 stages T009–T011** only.

---

## Notes

- Preconditions for VS-1: `make bundle-esptool` + `make sync-installer-firmware`  
- Flash Success ≠ network reachability  
- Do not add automated test tasks to this catalog  
- Commit only with explicit user approval (constitution) unless cloud-agent policy overrides  

**Constitution compliance:** Philarmony per `.specify/memory/constitution.md` v1.0.0 — manual catalog = this feature; automation on product specs
