# Makefile Targets Contract — Developer Tooling

**Scope**: Host-side orchestration only. Does not change WebSocket/HTTP firmware APIs.  
**Prerequisite**: PlatformIO Core (`pio`) on `PATH` (or via `~/.platformio/penv/bin`).

---

## Variables

| Variable | Default | Meaning |
|----------|---------|---------|
| `ENV` | `esp32devkitc` | PlatformIO environment for firmware build/flash/monitor |
| `PORT` / `UPLOAD_PORT` | (unset) | Optional serial device; when set, pass through to `pio` upload/monitor |

Allowed `ENV` values (must match `platformio.ini`): `esp32devkitc`, `lilygo_tdisplay_v1`, `esp32_2432s028`. Tests always use environment `native`.

---

## Targets

### `make` / `make build` / `make compile`
- **Action**: Build firmware for `$(ENV)`.
- **Command**: `pio run -e $(ENV)`
- **Success**: exit code 0; firmware binary under `.pio/build/$(ENV)/`.
- **Failure**: non-zero exit (compiler/link errors).

### `make test`
- **Action**: Run host Unity flow tests.
- **Command**: `pio test -e native`
- **Success**: all test cases passed, exit 0.
- **Failure**: any failed/errored suite → non-zero (CI must fail).

### `make flash` / `make upload` / `make install`
- **Action**: Build (if needed) and flash application image to connected ESP32 for `$(ENV)`.
- **Command**: `pio run -e $(ENV) -t upload` (+ upload port flags when `PORT`/`UPLOAD_PORT` set).
- **Aliases**: `flash`, `upload`, and `install` MUST be equivalent.
- **Success**: upload completed, exit 0.
- **Failure**: no device / wrong port / upload error → non-zero.

### `make uploadfs` (optional)
- **Action**: Upload LittleFS image for `$(ENV)`.
- **Command**: `pio run -e $(ENV) -t uploadfs`

### `make monitor`
- **Action**: Open serial monitor for `$(ENV)`.
- **Command**: `pio device monitor` (or `pio run -e $(ENV) -t monitor`) with baud from `platformio.ini` (115200).

### `make clean`
- **Action**: Remove build products for `$(ENV)` (and preferably `native` artifacts used by tests).
- **Command**: `pio run -e $(ENV) -t clean` (extend as needed for native).

### `make help`
- **Action**: Print available targets, defaults, and example `ENV=` usage.
- **Success**: exit 0; human-readable list (PT-BR or bilingual comments OK).

---

## Non-goals

- Replacing `platformio.ini` board/lib configuration.
- Cross-compiling without PlatformIO.
- Auto-committing or flashing without an attached device when `flash`/`install` is invoked.
