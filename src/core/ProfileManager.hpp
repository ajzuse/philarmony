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
 * ProfileManager - Built-in presets and custom profile management
 */
#pragma once

#include <ArduinoJson.h>
#include "ConfigManager.hpp"
#include "StateMachine.hpp"

namespace filament_dryer {

class ProfileManager {
public:
    explicit ProfileManager(ConfigManager& config_manager);

    std::vector<FilamentProfile> listProfiles() const;
    FilamentProfile getProfile(const String& profile_id) const;
    bool createProfile(FilamentProfile& profile);
    bool updateProfile(const FilamentProfile& profile);
    bool deleteProfile(const String& profile_id);
    void resetToDefaults();

    bool buildSessionFromRequest(const JsonObject& payload, DryingSession& session) const;

private:
    ConfigManager& config_manager_;
};

}  // namespace filament_dryer
