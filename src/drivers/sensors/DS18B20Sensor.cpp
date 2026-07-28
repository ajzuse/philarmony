/**
 * DS18B20Sensor - Implementation
 */
#include "DS18B20Sensor.hpp"

namespace filament_dryer {

DS18B20Sensor::DS18B20Sensor() {}

DS18B20Sensor::~DS18B20Sensor() {
    delete onewire_;
}

bool DS18B20Sensor::begin(const JsonObject& config) {
    gpio_pin_ = config["gpio_pin"] | 4;
    resolution_ = config["resolution"] | 12;
    if (resolution_ < 9) resolution_ = 9;
    if (resolution_ > 12) resolution_ = 12;
    
    onewire_ = new OneWire(gpio_pin_);
    
    if (findDevice()) {
        // Set resolution
        uint8_t res_reg = 0x1F | ((resolution_ - 9) << 5);
        onewire_->reset();
        onewire_->select(device_addr_);
        onewire_->write(0x4E); // Write Scratchpad
        onewire_->write(0x00); // TH
        onewire_->write(0x00); // TL
        onewire_->write(res_reg); // Configuration
        
        initialized_ = true;
    }
    
    return initialized_;
}

bool DS18B20Sensor::findDevice() {
    onewire_->reset_search();
    if (!onewire_->search(device_addr_)) {
        return false;
    }
    
    // Verify CRC
    if (OneWire::crc8(device_addr_, 7) != device_addr_[7]) {
        return false;
    }
    
    // Check family code (0x28 = DS18B20)
    if (device_addr_[0] != 0x28) {
        return false;
    }
    
    return true;
}

SensorReading DS18B20Sensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    
    if (!initialized_) {
        reading.valid = false;
        reading.error_message = "Not initialized";
        return reading;
    }
    
    // Start conversion
    onewire_->reset();
    onewire_->select(device_addr_);
    onewire_->write(0x44, 1); // Start conversion, parasite power
    
    // Wait for conversion (max 750ms for 12-bit)
    uint32_t wait_time = 94 << (resolution_ - 9);
    delay(wait_time);
    
    int16_t raw_temp;
    if (readScratchpad(raw_temp)) {
        reading.temperature = raw_temp / 16.0f;
        reading.humidity = NAN; // DS18B20 doesn't measure humidity
        reading.valid = true;
        last_reading_ = reading;
    } else {
        reading.valid = false;
        reading.error_message = "Failed to read scratchpad";
    }
    
    return reading;
}

bool DS18B20Sensor::readScratchpad(int16_t& raw_temp) {
    onewire_->reset();
    onewire_->select(device_addr_);
    onewire_->write(0xBE); // Read Scratchpad
    
    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        data[i] = onewire_->read();
    }
    
    // Verify CRC
    if (OneWire::crc8(data, 8) != data[8]) {
        return false;
    }
    
    raw_temp = (data[1] << 8) | data[0];
    return true;
}

bool DS18B20Sensor::isConnected() {
    return initialized_;
}

} // namespace filament_dryer