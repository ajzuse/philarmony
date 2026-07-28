# DEC-004: Safety & Fault Tolerance

**Tags:** safety, failsafe  
**Source:** research.md §5  
**Updated:** 2026-07-28

## Decision
Any anomaly → `FAULT_STOPPED` with heater forced LOW immediately; network/logging/display stay up.

## Key detectors
Sensor disconnect via `sensor_timeout_ms`, over-temp hard limit, thermal runaway, I2C recovery, NVS corrupt → factory defaults + philarmony AP.
