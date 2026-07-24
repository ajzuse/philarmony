/**
 * SHT31 I2C Temperature & Humidity Sensor Driver
 */
#pragma once

#include "IDriverInterfaces.hpp"
#include <Wire.h>

namespace filament_dryer {

class SHT31Sensor : public ISensorDriver {
public:
    SHT31Sensor();
    ~SHT31Sensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "sht31"; }
    String getName() const override { return "SHT31 I2C Temp+Humidity"; }
    bool isConnected() override;
    
private:
    TwoWire* wire_ = &Wire;
    uint8_t i2c_address_ = 0x44;
    uint8_t sda_pin_ = 21;
    uint8_t scl_pin_ = 22;
    uint8_t i2c_bus_ = 0;
    bool initialized_ = false;
    SensorReading last_reading_;
    
    // SHT31 commands
    static constexpr uint16_t CMD_MEAS_HIGHREP = 0x2C06;
    static constexpr uint16_t CMD_MEAS_MEDREP = 0x2C0D;
    static constexpr uint16_t CMD_MEAS_LOWREP = 0x2C10;
    static constexpr uint16_t CMD_READ_STATUS = 0xF32D;
    static constexpr uint16_t CMD_CLEAR_STATUS = 0x3041;
    static constexpr uint16_t CMD_SOFT_RESET = 0x30A2;
    static constexpr uint16_t CMD_HEATER_ENABLE = 0x306D;
    static constexpr uint16_t CMD_HEATER_DISABLE = 0x3066;
    
    bool writeCommand(uint16_t cmd);
    bool readData(uint16_t cmd, uint8_t* data, size_t len);
    uint8_t crc8(const uint8_t* data, size_t len);
    float calculateTemperature(uint16_t raw);
    float calculateHumidity(uint16_t raw);
};

} // namespace filament_dryer