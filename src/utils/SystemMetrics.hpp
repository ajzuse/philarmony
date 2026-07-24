/**
 * SystemMetrics - FreeRTOS CPU and Heap utilization monitoring
 */
#pragma once

#include <Arduino.h>

namespace filament_dryer {

class SystemMetrics {
public:
    SystemMetrics();
    ~SystemMetrics();
    
    bool begin(uint32_t sample_interval_ms = 1000);
    void end();
    
    // Get current metrics
    float getCpuUsagePercent() const { return cpu_usage_pct_; }
    uint32_t getFreeHeapBytes() const { return free_heap_bytes_; }
    uint32_t getMinFreeHeapBytes() const { return min_free_heap_bytes_; }
    uint32_t getLargestFreeBlockBytes() const { return largest_free_block_bytes_; }
    uint32_t getPsramFreeBytes() const { return psram_free_bytes_; }
    uint32_t getUptimeMs() const { return uptime_ms_; }
    
    // Update metrics (call periodically)
    void update();
    
    // Reset minimum free heap tracking
    void resetMinFreeHeap();
    
    // Get formatted status string
    String getStatusString() const;

private:
    bool initialized_ = false;
    uint32_t sample_interval_ms_ = 1000;
    uint32_t last_update_ms_ = 0;
    
    // CPU measurement (using FreeRTOS run time stats if available)
    float cpu_usage_pct_ = 0.0f;
    uint32_t last_idle_time_ = 0;
    uint32_t last_total_time_ = 0;
    
    // Memory metrics
    uint32_t free_heap_bytes_ = 0;
    uint32_t min_free_heap_bytes_ = 0xFFFFFFFF;
    uint32_t largest_free_block_bytes_ = 0;
    uint32_t psram_free_bytes_ = 0;
    
    // Uptime
    uint32_t uptime_ms_ = 0;
    uint32_t last_uptime_ms_ = 0;
    
    // For CPU calculation
    uint32_t last_update_time_ = 0;
};

} // namespace filament_dryer