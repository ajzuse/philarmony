# Control app packaging (DEC-011 patterns)

Artifacts land under `dist/` (gitignored). Prefer root Make targets:

```bash
make package-control-linux
make package-control-linux-appimage
make package-control-macos
make package-control-windows
```

## Scripts

| Script | Host | Output |
|--------|------|--------|
| `packaging/build-appimage.sh` | Linux | `dist/PhilarmonyControl-x86_64.AppImage` |
| `packaging/build-dmg.sh` | macOS | `dist/PhilarmonyControl-macos.dmg` |
| `packaging/build-msix.sh` | Windows | `dist/PhilarmonyControl-windows.msix` |

Each script expects `flutter build <platform> --release` to have run first (Make targets invoke both).

## Linux

`flutter build linux --release` → copy runner from `build/linux/*/release/bundle/` into `dist/control-linux/`, or run `build-appimage.sh` for AppImage.

## macOS / Windows

Host-OS builds only. DMG via `build-dmg.sh`; MSIX via `dart run msix:create` wrapped by `build-msix.sh` (unsigned OK for MVP — document SmartScreen).

## Store / background notes

Mobile background WebSocket requires Android FGS (`dataSync`) and iOS background modes — documented in app `README.md`.
