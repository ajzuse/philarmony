/*
 * Philarmony Filament Dryer ESP32 Firmware
 * Copyright (C) 2026 Philarmony Contributors
 *
 * Shared host/firmware timing contracts for SC-01 / 1Hz status (T163).
 */
#pragma once

#include <cstdint>

namespace filament_dryer {

/** Control loop period on Core 1 (50 Hz). */
constexpr uint32_t kControlLoopPeriodMs = 20;

/** Status/telemetry broadcast period (1 Hz ±100 ms → [900, 1100]). */
constexpr uint32_t kStatusBroadcastPeriodMs = 1000;

/** Control loops between status broadcasts. */
constexpr uint32_t kStatusLoopsPerBroadcast =
    kStatusBroadcastPeriodMs / kControlLoopPeriodMs;

/** SC-01: first STA attempt must fail-fast so AP is ready under 5s. */
constexpr uint32_t kWifiStaFailFastMaxMs = 5000;

/** First STA attempt timeout (must be < kWifiStaFailFastMaxMs). */
constexpr uint32_t kWifiStaConnectTimeoutMs = 4500;

/** Later reconnect attempts after a prior successful join. */
constexpr uint32_t kWifiStaReconnectTimeoutMs = 15000;

inline bool statusPeriodWithinScTolerance(uint32_t period_ms) {
    return period_ms >= 900 && period_ms <= 1100;
}

}  // namespace filament_dryer
