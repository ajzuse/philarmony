# UI fonts

Roboto Regular subsets compiled for LVGL 8 (`ui_font_roboto_{12,16,24,32}.c`).

- Source: [Google Fonts Roboto](https://github.com/googlefonts/roboto) (Apache License 2.0)
- Glyph range: U+0020–U+007E, U+00A0–U+00FF (ASCII + Latin-1 for PT-BR accents)
- bpp: 2

Regenerate:

```bash
npx --yes lv_font_conv --font Roboto-Regular.ttf --size 12 --bpp 2 --format lvgl \
  --lv-include lvgl.h --lv-font-name ui_font_roboto_12 \
  -r '0x20-0x7E,0xA0-0xFF' -o src/ui/assets/ui_font_roboto_12.c \
  --force-fast-kern-format
```
