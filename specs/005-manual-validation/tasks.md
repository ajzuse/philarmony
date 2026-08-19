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

## Phase 5: User Story 3 — Control-app & touchscreen manual stages (Priority: P2)

**Goal**: Catalog device/UI/packaging smokes for 003 (control app) and 004 (touchscreen) so product `tasks.md` stay free of open manuals

**Independent Test**: Each shipped control-app release gate has a VS-CTRL-* row here; results recorded in the 003 ledger

### Migrated from 003 quickstart (VS-1…VS-9 → VS-CTRL-1…VS-CTRL-9)

Ledger: `specs/003-filament-dryer-control-app/checklists/quickstart-validation.md`

- [x] T012 [US3] Expand placeholder into stage-named control-app manuals (see T016–T024); touchscreen remains T013
- [ ] T013 [US3] [MANUAL] **VS-UI-1 Touchscreen UI smoke** — when 004 ships P1 journeys, execute on-device UI smoke and record under `specs/004-esp32-touchscreen-ui/checklists/` (create checklist if missing)
- [ ] T016 [P] [US3] [MANUAL] **VS-CTRL-1 Discovery & connect** — Scan or manual host:80`/ws` → Connected → first `status/update` &lt;2s on real firmware/LAN; record in `specs/003-filament-dryer-control-app/checklists/quickstart-validation.md` (was 003 quickstart VS-1)
- [ ] T017 [P] [US3] [MANUAL] **VS-CTRL-2 Start / stop (no pause)** — Start cycle → drying → Stop with confirm; heater/fan off; confirm UI has no Pause/Resume; record in 003 checklist (was VS-2)
- [ ] T018 [US3] [MANUAL] **VS-CTRL-3 Config hardware** — Pin conflict blocked client-side; success/error topics per `001`; record in 003 checklist (was VS-3)
- [ ] T019 [P] [US3] [MANUAL] **VS-CTRL-4 History offline** — Complete a cycle → airplane/offline → list + detail from local store; record in 003 checklist (was VS-4)
- [ ] T020 [P] [US3] [MANUAL] **VS-CTRL-5 Adaptive layout** — Resize across ~600 / ~1024; chrome matches `contracts/adaptive-layout.md`; record in 003 checklist (was VS-5)
- [ ] T021 [US3] [MANUAL] **VS-CTRL-6 Multi-device** — Two `KnownDevice`s → switcher updates session; max concurrent enforced; record in 003 checklist (was VS-6)
- [ ] T022 [P] [US3] [MANUAL] **VS-CTRL-7 History export CSV + PDF** — Same completed cycle → CSV and PDF both openable; PDF has summary (+ charts when samples exist); record in 003 checklist (was VS-7)
- [ ] T023 [US3] [MANUAL] **VS-CTRL-8 Mobile background WS** — Active cycle → background app ≥2 min on **Android and iOS** → status continues or reconnects; complete/fault notification when permitted; record in 003 checklist (was VS-8)
- [ ] T024 [P] [US3] [MANUAL] **VS-CTRL-9 Packaging smoke (desktop)** — `make package-control-linux` (and/or macOS/Windows targets) → launch artifact; record in 003 checklist (was VS-9)

**Checkpoint**: No open `[MANUAL]` tasks on `specs/003-filament-dryer-control-app/tasks.md` or `004` open lists

---

## Phase 6: Polish & Cross-Cutting

- [x] T014 Sync README roadmap / 005 section for manual-validation catalog in `README.md`
- [ ] T015 [MANUAL] **VS-AUDIT Catalog audit** — re-scan `specs/*/tasks.md` for open manual remnants outside this file; fix or move if found

---

## Dependencies & Execution Order

```text
Done: T001–T008, T012, T014 (policy + 003 migration scaffolding)
Open manuals (installer): T009 (VS-1) then T010 // T011 (VS-4 // VS-5)
Open manuals (control app): T016…T024 (VS-CTRL-*) — prioritize T016, T017, T022, T023 for MVP gate
Future touchscreen: T013 (VS-UI-1)
Polish: T015 audit after stage closures
```

### Parallel Opportunities

- T010 // T011 (different host OS families)
- T016 // T017 // T019 // T020 // T022 // T024 (different control-app surfaces / hosts)
- T013 independent of control-app stages

---

## Implementation Strategy

### MVP close-out (installer release confidence)

1. T009 — VS-1 real ESP32 flash smoke  
2. T010 + T011 — VS-4 / VS-5 host packaging smoke  

### Control-app MVP manual gate (003 clarify locks)

1. T016 — VS-CTRL-1 discovery/connect  
2. T017 — VS-CTRL-2 start/stop  
3. T022 — VS-CTRL-7 CSV+PDF  
4. T023 — VS-CTRL-8 background WS (iOS+Android)  
5. Remaining T018–T021, T024 as feature completeness  

### Suggested MVP Scope

**Installer**: US2 stages T009–T011  
**Control app (when shipping 003 MVP)**: T016, T017, T022, T023  

---

## Notes

- Preconditions for installer VS-1: `make bundle-esptool` + `make sync-installer-firmware`  
- Flash Success ≠ network reachability  
- Control-app stage IDs use **VS-CTRL-*** to avoid colliding with installer **VS-1 / VS-4 / VS-5**  
- Do not add automated test tasks to this catalog  
- Commit only with explicit user approval (constitution) unless cloud-agent policy overrides  

**Constitution compliance:** Philarmony per `.specify/memory/constitution.md` v1.0.0 — manual catalog = this feature; automation on product specs
