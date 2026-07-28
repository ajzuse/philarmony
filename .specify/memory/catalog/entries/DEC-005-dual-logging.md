# DEC-005: Dual Logging

**Tags:** logging, littlefs  
**Source:** research.md §6  
**Updated:** 2026-07-28

`system.log` boot/ops; `drying.log` truncated at cycle start and preserved after stop. HTTP `/log/*` + WS `logs/stream`.
