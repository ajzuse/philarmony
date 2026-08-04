# Requirements checklist: 005 Manual Validation Catalog

**Feature**: `005-manual-validation`  
**Updated**: 2026-08-04 (v1.0.0 — policy inversion: manual-only catalog)  
**Spec**: `specs/005-manual-validation/spec.md`

## Content quality

- [x] Spec states sole ownership of open **manual** validation tasks
- [x] Spec states **automated** tests remain on product specs 001–004
- [x] Migrated installer stages named VS-1 / VS-4 / VS-5
- [x] Checklist ledger path for 002 documented
- [x] Out of scope excludes moving automation into this catalog

## Policy / constitution alignment

- [x] Constitution v1.0.0 encodes inverted split (manual→005, automation→product)
- [x] Product-spec banners updated (001–004)
- [x] `tasks-template.md` Tests guidance updated
- [x] Active feature set to `002-esp32-desktop-installer` in `.specify/feature.json`
- [x] Open manuals removed from `002` tasks (pointers to 005 T009–T011)

## Traceability

- [x] Former 002 T031 → 005 T009 (VS-1)
- [x] Former 002 T062 → 005 T010 (VS-4) + T011 (VS-5)
- [x] Results still recorded in `specs/002-esp32-desktop-installer/checklists/quickstart-validation.md`
