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
