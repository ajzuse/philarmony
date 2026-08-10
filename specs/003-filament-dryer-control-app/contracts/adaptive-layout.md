# Contract: Adaptive Layout (Phone / Tablet / Desktop)

**Feature**: `003-filament-dryer-control-app` | **Date**: 2026-08-10

## Goal

One widget tree for features; **shell** adapts. Feature screens MUST NOT fork into `mobile_dashboard.dart` / `desktop_dashboard.dart` unless a platform API forces it.

## Breakpoints

| Token | Logical width | Navigation | Content |
|-------|---------------|------------|---------|
| `compact` | &lt; 600 | Bottom `NavigationBar` | Single pane |
| `medium` | 600–1023 | `NavigationRail` | Single pane (wider padding) |
| `expanded` | ≥ 1024 | `NavigationRail` (+ optional top app bar denser) | Master–detail where listed |

Orientation: both portrait and landscape supported; width drives chrome, not platform.

## Destinations (shared)

1. Dashboard  
2. History  
3. Devices (discover + registry)  
4. Settings (includes device config entry when connected)

Cycle start can be modal/route from Dashboard; Config hardware as nested route under Settings or Dashboard overflow — same routes on all sizes.

## Expanded master–detail

| Route | Master | Detail |
|-------|--------|--------|
| Dashboard | Status cards + controls | Live charts |
| History | Cycle list | Cycle detail + charts |
| Devices | Known + discovered list | Connection / edit form |

Compact/medium: push detail as full-screen route.

## Shared building blocks

- `AdaptiveScaffold` — selects nav chrome  
- `AppBreakpoint` — `of(context)`  
- Feature widgets accept constraints; use `Wrap` / responsive grids, not platform checks  

Forbidden: `if (Platform.isAndroid)` inside feature UI for layout. Allowed: platform adapters for mDNS/notifications behind interfaces.
