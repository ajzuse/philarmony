# Quickstart validation (002)

Date: 2026-07-29  
**Manual stages owned by**: `specs/005-manual-validation` (T009 VS-1, T010 VS-4, T011 VS-5) — record PASS/PENDING here.

| Scenario | Result | Notes |
|----------|--------|-------|
| VS-1 First flash | PENDING | Needs hardware; run `make bundle-esptool` + `make sync-installer-firmware` then flash on Device→…→Install |
| VS-2 Pin conflict | PASS | `make test-flutter` (wizard_flow_test) |
| VS-3 Export/import | PASS | ProfileStore redaction + file export/import UI |
| VS-4 Host Win/macOS | PENDING | MSIX / DMG scripts ready; smoke on host/CI |
| VS-5 Host Linux | PENDING | AppImage/deb/rpm scripts ready; needs Linux runner |
| VS-6 Retry | PASS (code) | FlashStep Retry messaging; needs HW for E2E |
| VS-7 Local reconfigure | PASS (code) | LastProfileStore + Home New/Load/Import |
| VS-8 CI / automated | PASS | `make test-flutter` (core + app + last_profile + filament validator) |

## VS-1 hardware smoke checklist

- [ ] `make bundle-esptool` on host OS
- [ ] `make sync-installer-firmware`
- [ ] `make run` → select port → complete wizard → Flash Success
- [ ] Soft network check shows warn or ok (never fails flash)
- [ ] Record ESP32 model / port / host OS here when done

FVM: Flutter **3.44.0** (`.fvmrc`). Runners: `macos/`, `windows/`, `linux/` via `make create-platforms`.
