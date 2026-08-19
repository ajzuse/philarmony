#pragma once

#include "Arduino.h"
#include <algorithm>
#include <cstring>
#include <map>
#include <string>

inline std::map<std::string, std::string>& test_preferences_storage() {
    static std::map<std::string, std::string> storage;
    return storage;
}

inline void test_reset_preferences() { test_preferences_storage().clear(); }

class Preferences {
public:
    bool begin(const char*, bool = false) { return true; }
    void end() {}

    bool putString(const char* key, const String& value) {
        test_preferences_storage()[key] = std::string(value.c_str());
        return true;
    }

    String getString(const char* key, const String& default_value = String()) const {
        auto it = test_preferences_storage().find(key);
        return it == test_preferences_storage().end()
                   ? default_value
                   : String(it->second.c_str());
    }

    size_t putBytes(const char* key, const void* value, size_t len) {
        test_preferences_storage()[key] =
            std::string(reinterpret_cast<const char*>(value), len);
        return len;
    }

    size_t getBytesLength(const char* key) const {
        auto it = test_preferences_storage().find(key);
        return it == test_preferences_storage().end() ? 0 : it->second.size();
    }

    size_t getBytes(const char* key, void* buf, size_t max_len) const {
        auto it = test_preferences_storage().find(key);
        if (it == test_preferences_storage().end()) {
            return 0;
        }
        size_t len = std::min(max_len, it->second.size());
        std::memcpy(buf, it->second.data(), len);
        return len;
    }

    bool remove(const char* key) {
        return test_preferences_storage().erase(key) > 0;
    }
    bool clear() {
        test_preferences_storage().clear();
        return true;
    }
};
