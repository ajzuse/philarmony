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

#include "CycleHistoryStore.hpp"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <cmath>

namespace filament_dryer {

bool CycleHistoryStore::begin() {
    records_.clear();
    records_.reserve(kCapacity);
    next_id_ = 1;
    if (!LittleFS.begin()) {
        return false;
    }
    if (!LittleFS.exists(kPath)) {
        return true;
    }
    return load();
}

bool CycleHistoryStore::appendFromSession(const DryingSession& session) {
    CycleRecord rec;
    rec.id = next_id_++;
    rec.timestamp_ms = millis();
    rec.material_id = session.profile_id;
    rec.target_temp_c = session.target_temp_c;
    rec.avg_temp_c = session.current_temp_c;
    rec.max_temp_c = session.current_temp_c;
    rec.target_humidity_pct = session.target_humidity_pct;
    rec.avg_humidity_pct = session.current_humidity_pct;
    rec.duration_sec = session.elapsed_sec;
    rec.stop_reason = StateMachine::stopReasonToString(session.stop_reason);

    if (records_.size() >= kCapacity) {
        records_.erase(records_.begin());
    }
    records_.push_back(rec);
    return persist();
}

std::vector<CycleRecord> CycleHistoryStore::page(size_t offset, size_t limit) const {
    std::vector<CycleRecord> out;
    if (records_.empty() || limit == 0) {
        return out;
    }
    // Newest last in vector → iterate reverse
    size_t newest_index = records_.size() - 1;
    if (offset >= records_.size()) {
        return out;
    }
    size_t start = newest_index - offset;
    for (size_t i = 0; i < limit; ++i) {
        if (offset + i >= records_.size()) break;
        size_t idx = newest_index - (offset + i);
        out.push_back(records_[idx]);
        (void)start;
    }
    return out;
}

bool CycleHistoryStore::getById(uint32_t id, CycleRecord& out) const {
    for (const auto& r : records_) {
        if (r.id == id) {
            out = r;
            return true;
        }
    }
    return false;
}

bool CycleHistoryStore::load() {
    File file = LittleFS.open(kPath, FILE_READ);
    if (!file) return false;
    const String json = file.readString();
    file.close();
    if (json.isEmpty()) return true;

    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok ||
        !doc.is<JsonArray>()) {
        return false;
    }

    records_.clear();
    uint32_t highest_id = 0;
    for (JsonObject obj : doc.as<JsonArray>()) {
        CycleRecord record;
        record.id = obj["id"] | 0u;
        record.timestamp_ms = obj["timestamp_ms"] | 0u;
        record.material_id = obj["material_id"] | "";
        record.target_temp_c = obj["target_temp_c"] | 0.0f;
        record.avg_temp_c = obj["avg_temp_c"] | 0.0f;
        record.max_temp_c = obj["max_temp_c"] | 0.0f;
        record.target_humidity_pct = obj["target_humidity_pct"] | 0.0f;
        record.avg_humidity_pct = obj["avg_humidity_pct"] | 0.0f;
        record.duration_sec = obj["duration_sec"] | 0u;
        record.stop_reason = obj["stop_reason"] | "";
        if (record.id == 0) continue;
        if (records_.size() >= kCapacity) {
            records_.erase(records_.begin());
        }
        records_.push_back(record);
        if (record.id > highest_id) highest_id = record.id;
    }
    next_id_ = highest_id + 1;
    if (next_id_ == 0) next_id_ = 1;
    return true;
}

bool CycleHistoryStore::persist() const {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    for (const CycleRecord& record : records_) {
        JsonObject obj = array.add<JsonObject>();
        obj["id"] = record.id;
        obj["timestamp_ms"] = record.timestamp_ms;
        obj["material_id"] = record.material_id;
        obj["target_temp_c"] =
            std::isfinite(record.target_temp_c) ? record.target_temp_c : 0.0f;
        obj["avg_temp_c"] =
            std::isfinite(record.avg_temp_c) ? record.avg_temp_c : 0.0f;
        obj["max_temp_c"] =
            std::isfinite(record.max_temp_c) ? record.max_temp_c : 0.0f;
        obj["target_humidity_pct"] =
            std::isfinite(record.target_humidity_pct)
                ? record.target_humidity_pct
                : 0.0f;
        obj["avg_humidity_pct"] =
            std::isfinite(record.avg_humidity_pct)
                ? record.avg_humidity_pct
                : 0.0f;
        obj["duration_sec"] = record.duration_sec;
        obj["stop_reason"] = record.stop_reason;
    }

    String json;
    serializeJson(doc, json);
    File file = LittleFS.open(kPath, FILE_WRITE);
    if (!file) return false;
    const size_t written = file.print(json);
    file.close();
    return written == json.length();
}

} // namespace filament_dryer
