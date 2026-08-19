#pragma once

#include "FS.h"
#include <map>
#include <string>

inline std::map<std::string, std::string>& test_littlefs_storage() {
    static std::map<std::string, std::string> storage;
    return storage;
}

inline void test_reset_littlefs() { test_littlefs_storage().clear(); }

class LittleFSClass {
public:
    bool begin(bool = true, const char* = "/littlefs", uint8_t = 5, const char* = "spiffs") {
        return true;
    }

    bool exists(const char* path) const {
        return test_littlefs_storage().find(path) != test_littlefs_storage().end();
    }

    bool mkdir(const char*) { return true; }

    bool remove(const char* path) {
        return test_littlefs_storage().erase(path) > 0;
    }

    File open(const char* path, const char* mode) {
        if (mode && mode[0] == 'w') {
            test_littlefs_storage()[path] = "";
        }
        return File(String(path), mode);
    }
};

inline LittleFSClass LittleFS;

inline size_t File::size() const {
    auto it = test_littlefs_storage().find(path_.c_str());
    return it == test_littlefs_storage().end() ? 0 : it->second.size();
}

inline size_t File::read(uint8_t*, size_t) { return 0; }

inline size_t File::write(const uint8_t* buffer, size_t len) {
    auto& content = test_littlefs_storage()[path_.c_str()];
    content.append(reinterpret_cast<const char*>(buffer), len);
    return len;
}

inline size_t File::print(const String& data) {
    return write(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
}

inline bool File::seek(size_t) { return true; }

inline String File::readString() {
    auto it = test_littlefs_storage().find(path_.c_str());
    if (it == test_littlefs_storage().end()) {
        return String();
    }
    return String(it->second.c_str());
}
