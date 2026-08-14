# Control app packaging (DEC-011 patterns)

Artifacts land under `dist/` (gitignored). Prefer root Make targets:

```bash
make package-control-linux
make package-control-macos   # macOS host
make package-control-windows # Windows host
```

## Linux

`flutter build linux --release` → copy runner from `build/linux/*/release/bundle/` into `dist/control-linux/`.

## macOS / Windows

Host-OS builds only (`flutter build macos` / `flutter build windows`). See Make targets.

## Store / background notes

Mobile background WebSocket requires Android FGS (`dataSync`) and iOS background modes — documented in app `README.md`.
