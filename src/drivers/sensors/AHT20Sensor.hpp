/**
 * AHT20/AHT10 Sensor Driver - I2C Temperature + Humidity
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <Wire.h>

namespace filament_dryer {

class AHT20Sensor : public ISensorDriver {
public:
    AHT20Sensor();
    ~AHT20Sensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "aht20"; }
    String getName() const override { return "AHT20/AHT10 Temp+Humidity"; }
    bool isConnected() override;

private:
    TwoWire* wire_ = &Wire;
    uint8_t i2c_address_ = 0x38;
    int8_t sda_pin_ = 21;
    int8_t scl_pin_ = 22;
    uint8_t i2c_bus_ = 0;
    bool initialized_ = false;
    SensorReading last_reading_;
    uint32_t last_read_time_ = 0;

    static constexpr uint8_t CMD_INIT = 0xBE;
    static constexpr uint8_t CMD_MEASURE = 0xAC;
    static constexpr uint8_t CMD_SOFT_RESET = 0xBA;

    bool sendCommand(uint8_t cmd, const uint8_t* data = nullptr, size_t len = 0);
    bool readData(uint8_t* data, size_t len);
    float calcTemperature(uint32_t raw);
    float calcHumidity(uint32_t raw);
    bool waitForReady(uint32_t timeout_ms = 100);
};

} // namespace filament_dryer