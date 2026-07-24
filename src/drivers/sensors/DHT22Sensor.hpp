/**
 * DHT22 Digital Temperature & Humidity Sensor Driver
 */
#pragma once

#include "IDriverInterfaces.hpp"

namespace filament_dryer {

class DHT22Sensor : public ISensorDriver {
public:
    DHT22Sensor();
    ~DHT22Sensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override { return "dht22"; }
    String getName() const override { return "DHT22 1-Wire Temp+Humidity"; }
    bool isConnected() override;
    
private:
    int8_t gpio_pin_ = 4;
    bool initialized_ = false;
    SensorReading last_reading_;
    uint32_t last_read_time_ = 0;
    
    // DHT22 timing
    static constexpr uint32_t MIN_READ_INTERVAL_MS = 2000;
    
    bool readData(uint8_t* data);
    float calculateTemperature(int16_t raw);
    float calculateHumidity(int16_t raw);
};

} // namespace filament_dryer