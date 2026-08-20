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
 * CycleHistoryStore - circular buffer of on-device cycle records (max 50)
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "../../core/StateMachine.hpp"

namespace filament_dryer {

struct CycleRecord {
    static constexpr size_t kMaxSamples = 32;

    uint32_t id = 0;
    uint32_t timestamp_unix = 0;
    String material_id;
    float target_temp_c = 0.0f;
    float avg_temp_c = 0.0f;
    float max_temp_c = 0.0f;
    float target_humidity_pct = 0.0f;
    float avg_humidity_pct = 0.0f;
    uint32_t duration_sec = 0;
    String stop_reason;
    uint8_t sample_count = 0;
    int16_t temp_c_x10[kMaxSamples] = {};
    int16_t humidity_x10[kMaxSamples] = {};
};

class CycleHistoryStore {
public:
    static constexpr size_t kCapacity = 50;
    static constexpr size_t kInitialPage = 10;

    bool begin();
    void resetSampler();
    void recordSample(float temp_c, float humidity_pct);
    bool appendFromSession(const DryingSession& session);
    size_t size() const { return records_.size(); }
    /** Newest-first page; offset 0 = newest. */
    std::vector<CycleRecord> page(size_t offset, size_t limit) const;
    bool getById(uint32_t id, CycleRecord& out) const;
    String toCsv(const CycleRecord& record) const;
    bool fillExportJson(const CycleRecord& record, JsonObject obj) const;
    bool writeCsvToSd(const CycleRecord& record) const;

private:
    static constexpr const char* kPath = "/cycle_history.json";
    std::vector<CycleRecord> records_;
    uint32_t next_id_ = 1;
    float sample_sum_temp_ = 0.0f;
    float sample_sum_rh_ = 0.0f;
    float sample_max_temp_ = -1000.0f;
    uint32_t sample_n_ = 0;
    uint8_t live_count_ = 0;
    int16_t live_temp_x10_[CycleRecord::kMaxSamples] = {};
    int16_t live_rh_x10_[CycleRecord::kMaxSamples] = {};

    bool load();
    bool persist() const;
    void compactLiveSamples();
};

}  // namespace filament_dryer
