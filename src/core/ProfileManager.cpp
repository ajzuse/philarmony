#include "ProfileManager.hpp"

#include <Arduino.h>

namespace filament_dryer {

ProfileManager::ProfileManager(ConfigManager& config_manager)
    : config_manager_(config_manager) {}

std::vector<FilamentProfile> ProfileManager::listProfiles() const {
    return config_manager_.getProfiles();
}

FilamentProfile ProfileManager::getProfile(const String& profile_id) const {
    return config_manager_.getProfile(profile_id);
}

bool ProfileManager::createProfile(FilamentProfile& profile) {
    if (profile.id.isEmpty()) {
        char buf[24];
        snprintf(buf, sizeof(buf), "custom-%08lx", static_cast<unsigned long>(millis()));
        profile.id = buf;
    }
    profile.is_builtin = false;
    return config_manager_.addProfile(profile);
}

bool ProfileManager::updateProfile(const FilamentProfile& profile) {
    if (profile.id.isEmpty()) {
        return false;
    }
    return config_manager_.updateProfile(profile);
}

bool ProfileManager::deleteProfile(const String& profile_id) {
    if (profile_id.isEmpty()) {
        return false;
    }
    return config_manager_.deleteProfile(profile_id);
}

void ProfileManager::resetToDefaults() {
    config_manager_.resetProfilesToDefaults();
}

bool ProfileManager::buildSessionFromRequest(const JsonObject& payload,
                                             DryingSession& session) const {
    FilamentProfile profile;
    const String profile_id = payload["profile_id"] | "";

    if (!profile_id.isEmpty()) {
        profile = getProfile(profile_id);
        if (profile.id.isEmpty()) {
            return false;
        }
        session.profile_id = profile.id;
        session.target_temp_c = profile.target_temp_c;
        session.max_duration_min = profile.default_duration_min;
        session.target_humidity_pct = profile.target_humidity_pct;
    } else {
        if (!payload["target_temp_c"].is<float>() || !payload["max_duration_min"].is<uint16_t>()) {
            return false;
        }
        session.profile_id = "custom";
        session.target_temp_c = payload["target_temp_c"].as<float>();
        session.max_duration_min = payload["max_duration_min"].as<uint16_t>();
        session.target_humidity_pct = payload["target_humidity_pct"] | NAN;
    }

    if (payload["target_temp_c"].is<float>()) {
        session.target_temp_c = payload["target_temp_c"].as<float>();
    }
    if (payload["max_duration_min"].is<uint16_t>()) {
        session.max_duration_min = payload["max_duration_min"].as<uint16_t>();
    }
    if (payload["target_humidity_pct"].is<float>()) {
        session.target_humidity_pct = payload["target_humidity_pct"].as<float>();
    }

    const bool has_humidity = !isnan(session.target_humidity_pct);
    return ConfigManager::validateStartParams(session.target_temp_c, session.max_duration_min,
                                              has_humidity ? session.target_humidity_pct : 15.0f,
                                              has_humidity);
}

}  // namespace filament_dryer
