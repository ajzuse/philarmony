# Implementation Plan: Manual Validation Catalog

**Branch**: `005-manual-validation` | **Date**: 2026-08-04 | **Spec**: `specs/005-manual-validation/spec.md`  
**Constitution**: v1.0.0 — Catálogo Único de Validações Manuais

## Summary

Own all open **manual** validation stages (VS-*) for Philarmony. Product specs keep automation.

- Installer: VS-1 / VS-4 / VS-5 migrated from 002 T031/T062 → ledger `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md`
- Control app: **VS-CTRL-1…VS-CTRL-9** migrated from 003 quickstart VS-1…VS-9 → ledger `specs/003-filament-dryer-control-app/checklists/quickstart-validation.md`
- Touchscreen: **VS-UI-1** placeholder until 004 ships

## Technical Context

- **Type**: Process/catalog specification (no runtime product code)
- **Primary artifacts**: `tasks.md`, product checklists under `specs/00x-*/checklists/`
- **Active product feature**: `002-esp32-desktop-installer` (implementation complete; manuals here)

## Constitution Check

- Manual tasks only in this feature ✅  
- Automation remains on product specs ✅  
- Stage-aligned task names (VS-*) ✅  
- README + constitution synced ✅  

## Project Structure

```text
specs/005-manual-validation/
├── spec.md
├── plan.md
├── tasks.md
└── checklists/requirements.md
```

## Implementation Approach

1. Policy scaffolding (done): rename catalog, constitution v1.0.0, product banners, migrate 002 manuals  
2. Execute human stages (installer): T009 VS-1 → T010 VS-4 // T011 VS-5  
3. Control-app manuals registered (2026-08-11): T016–T024 **VS-CTRL-***; execute when shipping 003 MVP  
4. Touchscreen: T013 **VS-UI-1** when 004 needs smoke  
5. Periodic T015 audit for open manuals outside this catalog  

## Testing

Automated CI gates are unchanged and owned by product specs. This plan does not add automated suites here.
