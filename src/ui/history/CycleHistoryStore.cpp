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
#include "../ui_format.hpp"
#include <LittleFS.h>
#include <cmath>
#include <cstring>

#if !defined(UNIT_TEST)
#include <SD.h>
#endif

namespace filament_dryer {

namespace {
int16_t toX10(float value) {
    if (!std::isfinite(value)) {
        return 0;
    }
    return static_cast<int16_t>(lroundf(value * 10.0f));
}

void writeSamples(JsonObject obj, const char* key, const int16_t* values,
                  uint8_t count) {
    JsonArray array = obj[key].to<JsonArray>();
    for (uint8_t i = 0; i < count; ++i) {
        array.add(values[i]);
    }
}

void readSamples(JsonObject obj, const char* key, int16_t* values,
                 uint8_t& count) {
    if (!obj[key].is<JsonArray>()) {
        return;
    }
    for (JsonVariant v : obj[key].as<JsonArray>()) {
        if (count >= CycleRecord::kMaxSamples) {
            break;
        }
        values[count++] = static_cast<int16_t>(v.as<int>());
    }
}
}  // namespace

bool CycleHistoryStore::begin() {
    records_.clear();
    records_.reserve(kCapacity);
    next_id_ = 1;
    resetSampler();
    if (!LittleFS.begin()) {
        return false;
    }
    if (!LittleFS.exists(kPath)) {
        return true;
    }
    return load();
}

void CycleHistoryStore::resetSampler() {
    sample_sum_temp_ = 0.0f;
    sample_sum_rh_ = 0.0f;
    sample_max_temp_ = -1000.0f;
    sample_n_ = 0;
    live_count_ = 0;
}

void CycleHistoryStore::compactLiveSamples() {
    if (live_count_ < CycleRecord::kMaxSamples) {
        return;
    }
    const uint8_t half = CycleRecord::kMaxSamples / 2;
    for (uint8_t i = 0; i < half; ++i) {
        live_temp_x10_[i] = static_cast<int16_t>(
            (live_temp_x10_[static_cast<size_t>(i) * 2] +
             live_temp_x10_[static_cast<size_t>(i) * 2 + 1]) /
            2);
        live_rh_x10_[i] = static_cast<int16_t>(
            (live_rh_x10_[static_cast<size_t>(i) * 2] +
             live_rh_x10_[static_cast<size_t>(i) * 2 + 1]) /
            2);
    }
    live_count_ = half;
}

void CycleHistoryStore::recordSample(float temp_c, float humidity_pct) {
    if (!std::isfinite(temp_c) && !std::isfinite(humidity_pct)) {
        return;
    }
    const float temp = std::isfinite(temp_c) ? temp_c : 0.0f;
    const float rh = std::isfinite(humidity_pct) ? humidity_pct : 0.0f;
    sample_sum_temp_ += temp;
    sample_sum_rh_ += rh;
    if (temp > sample_max_temp_) {
        sample_max_temp_ = temp;
    }
    ++sample_n_;
    compactLiveSamples();
    live_temp_x10_[live_count_] = toX10(temp);
    live_rh_x10_[live_count_] = toX10(rh);
    ++live_count_;
}

bool CycleHistoryStore::appendFromSession(const DryingSession& session) {
    CycleRecord rec;
    rec.id = next_id_++;
    rec.timestamp_unix = ui_format::unixNowSec();
    rec.material_id = session.profile_id;
    rec.target_temp_c = session.target_temp_c;
    rec.target_humidity_pct = session.target_humidity_pct;
    rec.duration_sec = session.elapsed_sec;
    rec.stop_reason = StateMachine::stopReasonToString(session.stop_reason);
    if (sample_n_ > 0) {
        rec.avg_temp_c = sample_sum_temp_ / static_cast<float>(sample_n_);
        rec.avg_humidity_pct = sample_sum_rh_ / static_cast<float>(sample_n_);
        rec.max_temp_c = sample_max_temp_;
        rec.sample_count = live_count_;
        memcpy(rec.temp_c_x10, live_temp_x10_, sizeof(live_temp_x10_));
        memcpy(rec.humidity_x10, live_rh_x10_, sizeof(live_rh_x10_));
    } else {
        rec.avg_temp_c = session.current_temp_c;
        rec.max_temp_c = session.current_temp_c;
        rec.avg_humidity_pct = session.current_humidity_pct;
        rec.sample_count = 3;
        rec.temp_c_x10[0] = toX10(session.target_temp_c);
        rec.temp_c_x10[1] = toX10(session.current_temp_c);
        rec.temp_c_x10[2] = toX10(session.current_temp_c);
        rec.humidity_x10[0] = toX10(session.target_humidity_pct);
        rec.humidity_x10[1] = toX10(session.current_humidity_pct);
        rec.humidity_x10[2] = toX10(session.current_humidity_pct);
    }

    if (records_.size() >= kCapacity) {
        records_.erase(records_.begin());
    }
    records_.push_back(rec);
    resetSampler();
    return persist();
}

std::vector<CycleRecord> CycleHistoryStore::page(size_t offset,
                                                 size_t limit) const {
    std::vector<CycleRecord> out;
    if (records_.empty() || limit == 0) {
        return out;
    }
    size_t newest_index = records_.size() - 1;
    if (offset >= records_.size()) {
        return out;
    }
    for (size_t i = 0; i < limit; ++i) {
        if (offset + i >= records_.size()) {
            break;
        }
        size_t idx = newest_index - (offset + i);
        out.push_back(records_[idx]);
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

String CycleHistoryStore::toCsv(const CycleRecord& record) const {
    auto num = [](float value) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.2f", static_cast<double>(value));
        return String(buf);
    };
    String csv =
        "id,timestamp_unix,material,target_temp_c,avg_temp_c,max_temp_c,"
        "target_humidity_pct,avg_humidity_pct,duration_sec,stop_reason\n";
    csv += String(record.id) + "," + String(record.timestamp_unix) + "," +
           record.material_id + "," + num(record.target_temp_c) + "," +
           num(record.avg_temp_c) + "," + num(record.max_temp_c) + "," +
           num(record.target_humidity_pct) + "," + num(record.avg_humidity_pct) +
           "," + String(record.duration_sec) + "," + record.stop_reason + "\n";
    csv += "sample_index,temp_c,humidity_pct\n";
    for (uint8_t i = 0; i < record.sample_count; ++i) {
        csv += String(i) + "," + num(record.temp_c_x10[i] / 10.0f) + "," +
               num(record.humidity_x10[i] / 10.0f) + "\n";
    }
    return csv;
}

bool CycleHistoryStore::fillExportJson(const CycleRecord& record,
                                       JsonObject obj) const {
    obj["topic"] = "history/export";
    obj["id"] = record.id;
    obj["timestamp_unix"] = record.timestamp_unix;
    obj["material_id"] = record.material_id;
    obj["target_temp_c"] = record.target_temp_c;
    obj["avg_temp_c"] = record.avg_temp_c;
    obj["max_temp_c"] = record.max_temp_c;
    obj["target_humidity_pct"] = record.target_humidity_pct;
    obj["avg_humidity_pct"] = record.avg_humidity_pct;
    obj["duration_sec"] = record.duration_sec;
    obj["stop_reason"] = record.stop_reason;
    writeSamples(obj, "temp_c_x10", record.temp_c_x10, record.sample_count);
    writeSamples(obj, "humidity_x10", record.humidity_x10, record.sample_count);
    return true;
}

bool CycleHistoryStore::writeCsvToSd(const CycleRecord& record) const {
#if defined(UNIT_TEST)
    (void)record;
    return false;
#else
    if (!SD.begin()) {
        return false;
    }
    const String name = "/cycle_" + String(record.id) + ".csv";
    File file = SD.open(name, FILE_WRITE);
    if (!file) {
        return false;
    }
    const String csv = toCsv(record);
    const size_t written = file.print(csv);
    file.close();
    return written == csv.length();
#endif
}

bool CycleHistoryStore::load() {
    File file = LittleFS.open(kPath, FILE_READ);
    if (!file) {
        return false;
    }
    const String json = file.readString();
    file.close();
    if (json.isEmpty()) {
        return true;
    }

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
        record.timestamp_unix =
            obj["timestamp_unix"] | (obj["timestamp_ms"] | 0u);
        record.material_id = obj["material_id"] | "";
        record.target_temp_c = obj["target_temp_c"] | 0.0f;
        record.avg_temp_c = obj["avg_temp_c"] | 0.0f;
        record.max_temp_c = obj["max_temp_c"] | 0.0f;
        record.target_humidity_pct = obj["target_humidity_pct"] | 0.0f;
        record.avg_humidity_pct = obj["avg_humidity_pct"] | 0.0f;
        record.duration_sec = obj["duration_sec"] | 0u;
        record.stop_reason = obj["stop_reason"] | "";
        uint8_t count = 0;
        int16_t temps[CycleRecord::kMaxSamples] = {};
        int16_t rhs[CycleRecord::kMaxSamples] = {};
        readSamples(obj, "temp_c_x10", temps, count);
        uint8_t rh_count = 0;
        readSamples(obj, "humidity_x10", rhs, rh_count);
        record.sample_count = count < rh_count ? count : rh_count;
        if (record.sample_count == 0 && count > 0) {
            record.sample_count = count;
        }
        memcpy(record.temp_c_x10, temps, sizeof(temps));
        memcpy(record.humidity_x10, rhs, sizeof(rhs));
        if (record.id == 0) {
            continue;
        }
        if (records_.size() >= kCapacity) {
            records_.erase(records_.begin());
        }
        records_.push_back(record);
        if (record.id > highest_id) {
            highest_id = record.id;
        }
    }
    next_id_ = highest_id + 1;
    if (next_id_ == 0) {
        next_id_ = 1;
    }
    return true;
}

bool CycleHistoryStore::persist() const {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    for (const CycleRecord& record : records_) {
        JsonObject obj = array.add<JsonObject>();
        obj["id"] = record.id;
        obj["timestamp_unix"] = record.timestamp_unix;
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
        writeSamples(obj, "temp_c_x10", record.temp_c_x10, record.sample_count);
        writeSamples(obj, "humidity_x10", record.humidity_x10,
                     record.sample_count);
    }

    String json;
    serializeJson(doc, json);
    File file = LittleFS.open(kPath, FILE_WRITE);
    if (!file) {
        return false;
    }
    const size_t written = file.print(json);
    file.close();
    return written == json.length();
}

}  // namespace filament_dryer
