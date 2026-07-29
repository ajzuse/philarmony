#pragma once

#include "Arduino.h"

#ifndef FILE_READ
#define FILE_READ "r"
#define FILE_APPEND "a"
#define FILE_WRITE "w"
#endif

class File {
public:
    File() = default;
    explicit File(const String& path, const char* mode = FILE_READ) : path_(path), mode_(mode) {}

    operator bool() const { return !path_.isEmpty(); }
    size_t size() const;
    size_t read(uint8_t* buffer, size_t len);
    size_t write(const uint8_t* buffer, size_t len);
    size_t print(const String& data);
    void close() {}
    bool seek(size_t pos);
    String readString();

private:
    String path_;
    const char* mode_ = FILE_READ;
};
