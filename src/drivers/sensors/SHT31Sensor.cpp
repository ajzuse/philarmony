/**
 * SHT31 I2C Temperature & Humidity Sensor Driver - Implementation
 */
#include "SHT31Sensor.hpp"

namespace filament_dryer {

SHT31Sensor::SHT31Sensor() {}

SHT31Sensor::~SHT31Sensor() {}

bool SHT31Sensor::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    // Parse configuration
    i2c_address_ = config["i2c_address"] | 0x44;
    sda_pin_ = config["sda_pin"] | 21;
    scl_pin_ = config["scl_pin"] | 22;
    i2c_bus_ = config["i2c_bus"] | 0;
    
    // Initialize I2C
    if (i2c_bus_ == 0) {
        wire_ = &Wire;
    } else {
        #if defined(ESP32)
        wire_ = &Wire1;
        #else
        wire_ = &Wire;
        #endif
    }
    
    wire_->begin(sda_pin_, scl_pin_);
    wire_->setClock(400000); // 400kHz fast mode
    
    // Soft reset
    if (!writeCommand(CMD_SOFT_RESET)) {
        Serial.println("[SHT31] ERROR: Soft reset failed");
        return false;
    }
    delay(10);
    
    // Clear status register
    writeCommand(CMD_CLEAR_STATUS);
    
    // Verify connection by reading status
    uint8_t status_data[3];
    if (!readData(CMD_READ_STATUS, status_data, 3)) {
        Serial.println("[SHT31] ERROR: Failed to read status register");
        return false;
    }
    
    initialized_ = true;
    Serial.printf("[SHT31] Initialized at 0x%02X (SDA=%d, SCL=%d)\n", 
                  i2c_address_, sda_pin_, scl_pin_);
    return true;
}

SensorReading SHT31Sensor::read() {
    SensorReading reading;
    reading.sensor_type = "sht31";
    reading.timestamp = millis();
    reading.valid = false;
    reading.temperature_c = NAN;
    reading.humidity_pct = NAN;
    
    if (!initialized_) {
        reading.error = "Not initialized";
        return reading;
    }
    
    // High repeatability measurement
    uint8_t data[6];
    if (!readData(CMD_MEAS_HIGHREP, data, 6)) {
        reading.error = "I2C read failed";
        return reading;
    }
    
    // Parse temperature (bytes 0-1) with CRC (byte 2)
    uint16_t temp_raw = (data[0] << 8) | data[1];
    if (crc8(data, 2) != data[2]) {
        reading.error = "Temperature CRC mismatch";
        return reading;
    }
    
    // Parse humidity (bytes 3-4) with CRC (byte 5)
    uint16_t hum_raw = (data[3] << 8) | data[4];
    if (crc8(data + 3, 2) != data[5]) {
        reading.error = "Humidity CRC mismatch";
        return reading;
    }
    
    reading.temperature_c = calculateTemperature(temp_raw);
    reading.humidity_pct = calculateHumidity(hum_raw);
    reading.valid = true;
    reading.error = "";
    
    last_reading_ = reading;
    return reading;
}

bool SHT31Sensor::isConnected() {
    if (!initialized_) return false;
    
    uint8_t status_data[3];
    return readData(CMD_READ_STATUS, status_data, 3);
}

bool SHT31Sensor::writeCommand(uint16_t cmd) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(cmd >> 8);
    wire_->write(cmd & 0xFF);
    return wire_->endTransmission() == 0;
}

bool SHT31Sensor::readData(uint16_t cmd, uint8_t* data, size_t len) {
    if (!writeCommand(cmd)) return false;
    
    // Wait for measurement (15ms for high repeatability)
    if (cmd == CMD_MEAS_HIGHREP) delay(15);
    else if (cmd == CMD_MEAS_MEDREP) delay(6);
    else if (cmd == CMD_MEAS_LOWREP) delay(4);
    else delay(1);
    
    wire_->requestFrom(i2c_address_, len);
    if (wire_->available() < len) return false;
    
    for (size_t i = 0; i < len; i++) {
        data[i] = wire_->read();
    }
    return true;
}

uint8_t SHT31Sensor::crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc <<= 1;
        }
    }
    return crc;
}

float SHT31Sensor::calculateTemperature(uint16_t raw) {
    return -45.0f + 175.0f * (float)raw / 65535.0f;
}

float SHT31Sensor::calculateHumidity(uint16_t raw) {
    return 100.0f * (float)raw / 65535.0f;
}

} // namespace filament_dryer