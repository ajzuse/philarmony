#pragma once

#include "Arduino.h"
#include <cstring>
#include <map>
#include <string>

class Preferences {
public:
    bool begin(const char*, bool = false) { return true; }
    void end() {}

    bool putString(const char* key, const String& value) {
        storage_[key] = std::string(value.c_str());
        return true;
    }

    String getString(const char* key, const String& default_value = String()) {
        auto it = storage_.find(key);
        return it == storage_.end() ? default_value : String(it->second.c_str());
    }

    bool putBytes(const char* key, const void* value, size_t len) {
        storage_[key] = std::string(reinterpret_cast<const char*>(value), len);
        return true;
    }

    size_t getBytes(const char* key, void* buf, size_t max_len) {
        auto it = storage_.find(key);
        if (it == storage_.end()) {
            return 0;
        }
        size_t len = std::min(max_len, it->second.size());
        std::memcpy(buf, it->second.data(), len);
        return len;
    }

    bool remove(const char* key) { return storage_.erase(key) > 0; }
    bool clear() {
        storage_.clear();
        return true;
    }

private:
    std::map<std::string, std::string> storage_;
};
