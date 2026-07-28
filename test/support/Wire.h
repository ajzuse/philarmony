#pragma once

#include "Arduino.h"

class TwoWire {
public:
    void begin(int = -1, int = -1) {}
    void setClock(uint32_t) {}
    void beginTransmission(uint8_t) {}
    size_t write(uint8_t) { return 1; }
    uint8_t endTransmission() { return 0; }
    size_t requestFrom(uint8_t, size_t) { return 0; }
    int available() { return 0; }
    int read() { return 0; }
};

inline TwoWire Wire;
inline TwoWire Wire1;
