# DEC-008: WiFi NVS Ownership & Fail-Fast STA

**Tags:** wifi, nvs, sc-01  
**Source:** Phase 17 converge (T108/T110)  
**Updated:** 2026-07-28

`ConfigManager` is the sole NVS writer for WiFi credentials. `WifiManager` reads/writes through it. First STA attempt times out at 4.5s (and hard-fails on NO_SSID/CONNECT_FAILED) so AP meets SC-01 <5s.
