/**
 * DS18B20 1-Wire Temperature Sensor Driver
 */
#pragma once

#include "IDriverInterfaces.hpp"
#include <OneWire.h>

namespace filament_dryer {

class DS18B20Sensor : public ISensorDriver {
public:
    DS18B20Sensor();
    ~DS18B20Sensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "ds18b20"; }
    String getName() const override { return "DS18B20 1-Wire Temperature"; }
    bool isConnected() override;
    
private:
    int8_t gpio_pin_ = 4;
    uint8_t resolution_ = 12; // 9-12 bits
    OneWire* onewire_ = nullptr;
    bool initialized_ = false;
    SensorReading last_reading_;
    uint8_t device_addr_[8] = {0};
    
    bool findDevice();
    bool readScratchpad(int16_t& raw_temp);
};

} // namespace filament_dryer