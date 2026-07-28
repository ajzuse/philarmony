/*
 * Philarmony Filament Dryer ESP32 Firmware
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * SystemMetrics - Implementation
 * FreeRTOS CPU and Heap utilization monitoring
 */
#include "SystemMetrics.hpp"

namespace filament_dryer {

SystemMetrics::SystemMetrics() {}

SystemMetrics::~SystemMetrics() {
    end();
}

bool SystemMetrics::begin(uint32_t sample_interval_ms) {
    if (initialized_) return true;
    
    sample_interval_ms_ = sample_interval_ms;
    last_update_ms_ = millis();
    last_update_time_ = millis();
    uptime_ms_ = 0;
    min_free_heap_bytes_ = ESP.getFreeHeap();
    
    initialized_ = true;
    
    return true;
}

void SystemMetrics::end() {
    initialized_ = false;
}

void SystemMetrics::update() {
    if (!initialized_) return;
    
    uint32_t now = millis();
    
    // Only update at sample interval
    if (now - last_update_ms_ < sample_interval_ms_) {
        return;
    }
    
    uint32_t dt = now - last_update_ms_;
    last_update_ms_ = now;
    uptime_ms_ += dt;
    
    // Memory metrics
    free_heap_bytes_ = ESP.getFreeHeap();
    min_free_heap_bytes_ = min(min_free_heap_bytes_, free_heap_bytes_);
    largest_free_block_bytes_ = ESP.getMaxAllocHeap();
    
    #ifdef ESP32
    psram_free_bytes_ = ESP.getFreePsram();
    #else
    psram_free_bytes_ = 0;
    #endif
    
    // CPU usage estimation based on task run time
    // Note: This is a simplified estimation. For accurate CPU usage,
    // FreeRTOS run time stats need to be enabled in sdkconfig
    uint32_t idle_time = 0;
    uint32_t total_time = 0;
    
    // If FreeRTOS run time stats are enabled, we could use:
    // vTaskGetRunTimeStats()
    // For now, we estimate based on loop time vs interval
    uint32_t loop_time = millis() - now;
    cpu_usage_pct_ = (float)loop_time / sample_interval_ms_ * 100.0f;
    cpu_usage_pct_ = constrain(cpu_usage_pct_, 0.0f, 100.0f);
}

void SystemMetrics::resetMinFreeHeap() {
    min_free_heap_bytes_ = ESP.getFreeHeap();
}

String SystemMetrics::getStatusString() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "CPU: %.1f%% | Heap: %lu/%lu bytes (min: %lu, max block: %lu) | PSRAM: %lu bytes | Uptime: %lus",
             cpu_usage_pct_,
             free_heap_bytes_, ESP.getHeapSize(),
             min_free_heap_bytes_, largest_free_block_bytes_,
             psram_free_bytes_,
             uptime_ms_ / 1000);
    return String(buffer);
}

} // namespace filament_dryer