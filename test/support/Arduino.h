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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

class String {
public:
    String() = default;
    String(const char* s) : data_(s ? s : "") {}
    String(const std::string& s) : data_(s) {}
    String(int value) : data_(std::to_string(value)) {}
    String(unsigned int value) : data_(std::to_string(value)) {}
    String(long value) : data_(std::to_string(value)) {}
    String(unsigned long value) : data_(std::to_string(value)) {}
    String(float value) : data_(std::to_string(value)) {}
    String(double value) : data_(std::to_string(value)) {}

    const char* c_str() const { return data_.c_str(); }
    size_t length() const { return data_.length(); }
    bool isEmpty() const { return data_.empty(); }
    void reserve(size_t n) { data_.reserve(n); }
    size_t write(uint8_t value) {
        data_.push_back(static_cast<char>(value));
        return 1;
    }
    size_t write(const uint8_t* values, size_t length) {
        if (!values) return 0;
        data_.append(reinterpret_cast<const char*>(values), length);
        return length;
    }
    int read() const {
        if (read_pos_ >= data_.size()) return -1;
        return static_cast<uint8_t>(data_[read_pos_++]);
    }

    String& operator=(const char* s) {
        data_ = s ? s : "";
        return *this;
    }
    String& operator=(const String& other) = default;
    String& operator+=(const char* s) {
        if (s) data_ += s;
        return *this;
    }
    String& operator+=(const String& other) {
        data_ += other.data_;
        return *this;
    }
    String& operator+=(char c) {
        data_.push_back(c);
        return *this;
    }

    friend String operator+(const String& lhs, const String& rhs) {
        return String(lhs.data_ + rhs.data_);
    }
    friend String operator+(const String& lhs, const char* rhs) {
        return String(lhs.data_ + (rhs ? rhs : ""));
    }
    friend String operator+(const char* lhs, const String& rhs) {
        return String(std::string(lhs ? lhs : "") + rhs.data_);
    }

    bool operator==(const char* rhs) const { return data_ == (rhs ? rhs : ""); }
    bool operator==(const String& rhs) const { return data_ == rhs.data_; }
    bool operator!=(const char* rhs) const { return !(*this == rhs); }
    bool operator!=(const String& rhs) const { return !(*this == rhs); }

    char operator[](size_t index) const { return data_[index]; }

    int indexOf(const char* substr) const {
        if (!substr) return -1;
        const auto pos = data_.find(substr);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }
    int indexOf(const String& substr) const { return indexOf(substr.c_str()); }

private:
    std::string data_;
    mutable size_t read_pos_ = 0;
};

inline String operator+(const String& lhs, float rhs) {
    return lhs + String(rhs);
}
inline String operator+(float lhs, const String& rhs) {
    return String(lhs) + rhs;
}

template <typename T>
T constrain(T value, T min_val, T max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

inline uint32_t& test_millis() {
    static uint32_t now = 0;
    return now;
}

inline void test_advance_millis(uint32_t delta) { test_millis() += delta; }
inline void test_set_millis(uint32_t value) { test_millis() = value; }

inline unsigned long millis() { return test_millis(); }
inline void delay(unsigned long) {}

#ifndef INPUT
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1
#endif

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline int digitalRead(int) { return HIGH; }
inline void delayMicroseconds(unsigned int) {}
inline void ledcSetup(int, int, int) {}
inline void ledcAttachPin(int, int) {}
inline void ledcWrite(int, int) {}

#include "ArduinoJsonString.hpp"
