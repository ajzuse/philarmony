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

#include <cstring>

#include "esp_freertos_hooks.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace filament_dryer {

namespace {

#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS

void updateCpuFromRunTimeStats(float& cpu_usage_pct,
                               uint32_t& last_idle_time,
                               uint32_t& last_total_time) {
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    if (task_count == 0) return;

    TaskStatus_t* status = static_cast<TaskStatus_t*>(
        pvPortMalloc(task_count * sizeof(TaskStatus_t)));
    if (!status) return;

    uint32_t total_runtime = 0;
    UBaseType_t populated =
        uxTaskGetSystemState(status, task_count, &total_runtime);

    uint32_t idle_runtime = 0;
    for (UBaseType_t i = 0; i < populated; ++i) {
        const char* name = status[i].pcTaskName;
        if (name && (std::strncmp(name, "IDLE", 4) == 0)) {
            idle_runtime += status[i].ulRunTimeCounter;
        }
    }
    vPortFree(status);

    if (total_runtime == 0 || total_runtime <= last_total_time) {
        last_idle_time = idle_runtime;
        last_total_time = total_runtime;
        return;
    }

    const uint32_t idle_delta = idle_runtime - last_idle_time;
    const uint32_t total_delta = total_runtime - last_total_time;
    last_idle_time = idle_runtime;
    last_total_time = total_runtime;

    const float idle_ratio =
        static_cast<float>(idle_delta) / static_cast<float>(total_delta);
    cpu_usage_pct = constrain((1.0f - idle_ratio) * 100.0f, 0.0f, 100.0f);
}

#else

// Idle/tick sampling — works without CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS.
volatile uint32_t g_idle_hits = 0;
volatile uint32_t g_tick_hits = 0;

bool IRAM_ATTR onIdleHook() {
    g_idle_hits++;
    // true → invoke at most once per FreeRTOS tick while the idle task runs
    return true;
}

void IRAM_ATTR onTickHook() {
    g_tick_hits++;
}

void updateCpuFromIdleHooks(float& cpu_usage_pct,
                            uint32_t& last_idle_time,
                            uint32_t& last_total_time) {
    const uint32_t idle = g_idle_hits;
    const uint32_t ticks = g_tick_hits;

    const uint32_t idle_delta = idle - last_idle_time;
    const uint32_t tick_delta = ticks - last_total_time;
    last_idle_time = idle;
    last_total_time = ticks;

    // Idle and tick hooks share counters across cores, so fully-idle
    // yields idle_delta ≈ tick_delta.
    if (tick_delta == 0) return;

    const float idle_ratio =
        static_cast<float>(idle_delta) / static_cast<float>(tick_delta);
    cpu_usage_pct = constrain((1.0f - idle_ratio) * 100.0f, 0.0f, 100.0f);
}

#endif

} // namespace

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

#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    last_idle_time_ = 0;
    last_total_time_ = 0;
#else
    // Register idle + tick hooks on all cores for portable CPU sampling.
    for (BaseType_t core = 0; core < portNUM_PROCESSORS; ++core) {
        esp_register_freertos_idle_hook_for_cpu(onIdleHook, core);
        esp_register_freertos_tick_hook_for_cpu(onTickHook, core);
    }
    hooks_registered_ = true;
    last_idle_time_ = g_idle_hits;
    last_total_time_ = g_tick_hits;
#endif

    initialized_ = true;
    return true;
}

void SystemMetrics::end() {
#if !CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    if (hooks_registered_) {
        esp_deregister_freertos_idle_hook(onIdleHook);
        esp_deregister_freertos_tick_hook(onTickHook);
        hooks_registered_ = false;
    }
#endif
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

#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    updateCpuFromRunTimeStats(cpu_usage_pct_, last_idle_time_, last_total_time_);
#else
    updateCpuFromIdleHooks(cpu_usage_pct_, last_idle_time_, last_total_time_);
#endif
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
