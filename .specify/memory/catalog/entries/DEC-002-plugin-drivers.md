# DEC-002: Plugin-Driver ObjectRegistry

**Tags:** architecture, drivers, plugins  
**Source:** research.md §2  
**Updated:** 2026-07-28

## Decision
Klipper-inspired "everything is a plugin": sensors, actuators, displays, and control algorithms load from `DriverRegistry` via JSON hardware config. Lifecycle hooks via `IPlugin`.

## Contracts
`ISensorDriver`, `IActuatorDriver`, `IDisplayDriver`, `IControlAlgorithm` plus plugin callbacks (`onInit`, `onSessionStart/Stop`, `onFault`, WS/HTTP handlers).
