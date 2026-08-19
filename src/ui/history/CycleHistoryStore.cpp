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

namespace filament_dryer {

bool CycleHistoryStore::begin() {
    records_.clear();
    records_.reserve(kCapacity);
    next_id_ = 1;
    return true;
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
    return true;
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

} // namespace filament_dryer
