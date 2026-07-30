# Bundled firmware flash set (from `make sync-installer-firmware`)

Place after PIO build:

- `firmware.bin` — app @ 0x10000
- `bootloader.bin` — @ 0x1000
- `partitions.bin` — @ 0x8000

```bash
make sync-installer-firmware
```

Gitignored `*.bin` — do not commit large binaries.
