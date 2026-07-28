#pragma once

#include <cstdint>

typedef int esp_err_t;
static constexpr esp_err_t ESP_OK = 0;

inline esp_err_t esp_task_wdt_init(uint32_t, bool) { return ESP_OK; }
inline esp_err_t esp_task_wdt_deinit() { return ESP_OK; }
inline esp_err_t esp_task_wdt_reset() { return ESP_OK; }
