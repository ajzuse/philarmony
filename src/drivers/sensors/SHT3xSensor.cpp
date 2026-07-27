/**
 * SHT3x I2C Temperature & Humidity Sensor Driver
 */
#include "SHT3xSensor.hpp"

namespace filament_dryer {

SHT3xSensor::SHT3xSensor() {}

SHT3xSensor::~SHT3xSensor() {}

bool SHT3xSensor::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    i2c_address_ = config["i2c_address"] | 0x44;
    i2c_bus_ = config["i2c_bus"] | 0;
    sda_pin_ = config["sda_pin"] | 21;
    scl_pin_ = config["scl_pin"] | 22;
    
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
    wire_->setClock(400000);
    
    // Soft reset
    if (!writeCommand(CMD_SOFT_RESET)) {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR, "SHT3x: Soft reset failed");
        return false;
    }
    delay(10);
    
    initialized_ = true;
    logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, 
                     "SHT3x initialized at 0x%02X (SDA=%d, SCL=%d)", 
                     i2c_address_, sda_pin_, scl_pin_);
    return true;
}

SensorReading SHT3xSensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    reading.valid = false;
    reading.temperature = NAN;
    reading.humidity = NAN;
    reading.pressure = NAN;
    
    if (!initialized_) {
        reading.error = "Not initialized";
        return reading;
    }
    
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
    
    reading.temperature = calculateTemperature(temp_raw);
    reading.humidity = calculateHumidity(hum_raw);
    reading.valid = true;
    reading.error = "";
    last_reading_ = reading;
    
    return reading;
}

bool SHT3xSensor::isConnected() {
    if (!initialized_) return false;
    
    uint8_t status[3];
    if (!readData(CMD_READ_STATUS, status, 3)) return false;
    return true;
}

bool SHT3xSensor::writeCommand(uint16_t cmd) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(cmd >> 8);
    wire_->write(cmd & 0xFF);
    return wire_->endTransmission() == 0;
}

bool SHT3xSensor::readData(uint16_t cmd, uint8_t* data, size_t len) {
    if (!writeCommand(cmd)) return false;
    
    // Wait for measurement
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

uint8_t SHT3xSensor::crc8(const uint8_t* data, size_t len) {
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

float SHT3xSensor::calculateTemperature(uint16_t raw) {
    return -45.0f + 175.0f * (float)raw / 65535.0f;
}

float SHT3xSensor::calculateHumidity(uint16_t raw) {
    return 100.0f * (float)raw / 65535.0f;
}

} // namespace filament_dryer