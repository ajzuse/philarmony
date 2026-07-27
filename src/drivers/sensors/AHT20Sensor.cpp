/**
 * AHT20Sensor - Implementation
 * AHT20/AHT10 I2C Temperature + Humidity Sensor
 */
#include "AHT20Sensor.hpp"

namespace filament_dryer {

AHT20Sensor::AHT20Sensor() {}

AHT20Sensor::~AHT20Sensor() {}

bool AHT20Sensor::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    i2c_address_ = config["i2c_address"] | 0x38;
    sda_pin_ = config["sda_pin"] | 21;
    scl_pin_ = config["scl_pin"] | 22;
    i2c_bus_ = config["i2c_bus"] | 0;

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
    wire_->setClock(400000);  // 400kHz fast mode

    // Soft reset
    if (!sendCommand(CMD_SOFT_RESET)) {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR, "AHT20: Soft reset failed");
        return false;
    }
    delay(20);

    // Initialize - normal mode
    uint8_t init_data[2] = {0x08, 0x00};  // Normal mode, calibration enabled
    if (!sendCommand(CMD_INIT, init_data, 2)) {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR, "AHT20: Initialization command failed");
        return false;
    }
    delay(100);

    // Wait for calibration bit
    for (int i = 0; i < 10; i++) {
        uint8_t status;
        if (readStatus(status) && (status & 0x08)) break;
        delay(10);
    }

    initialized_ = true;
    logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, 
                     "AHT20 initialized at 0x%02X (I2C bus %d, SDA=%d, SCL=%d)", 
                     i2c_address_, i2c_bus_, sda_pin_, scl_pin_);
    return true;
}

bool AHT20Sensor::sendCommand(uint8_t cmd, const uint8_t* data, size_t len) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(cmd);
    for (size_t i = 0; i < len; i++) {
        wire_->write(data[i]);
    }
    return wire_->endTransmission() == 0;
}

bool AHT20Sensor::readData(uint8_t* data, size_t len) {
    wire_->requestFrom(i2c_address_, len);
    size_t received = 0;
    uint32_t start = millis();
    while (received < len && millis() - start < 100) {
        if (wire_->available()) {
            data[received++] = wire_->read();
        }
    }
    return received == len;
}

bool AHT20Sensor::waitForReady(uint32_t timeout_ms) {
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        uint8_t status;
        if (readStatus(status)) {
            if ((status & 0x80) == 0) return true;  // Busy bit clear
        }
        delay(1);
    }
    return false;
}

bool AHT20Sensor::readStatus(uint8_t& status) {
    wire_->requestFrom(i2c_address_, 1);
    if (wire_->available()) {
        status = wire_->read();
        return true;
    }
    return false;
}

SensorReading AHT20Sensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    reading.valid = false;
    reading.temperature = NAN;
    reading.humidity = NAN;

    if (!initialized_) {
        reading.error = "Not initialized";
        return reading;
    }

    // Trigger measurement
    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    if (!sendCommand(CMD_MEASURE, cmd + 1, 2)) {
        reading.error = "Failed to trigger measurement";
        return reading;
    }

    // Wait for measurement to complete
    if (!waitForReady(100)) {
        reading.error = "Measurement timeout";
        return reading;
    }

    // Read 7 bytes: status + 3 temp + 3 humidity
    uint8_t data[7];
    if (!readData(data, 7)) {
        reading.error = "I2C read failed";
        return reading;
    }

    // Parse temperature (20-bit)
    uint32_t raw_temp = ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 8) | data[5];
    raw_temp >>= 4;  // Only 20 bits

    // Parse humidity (20-bit)
    uint32_t raw_hum = ((uint32_t)data[3] & 0x0F) << 16 | ((uint32_t)data[5] << 8) | data[6];

    reading.temperature = calcTemperature(raw_temp);
    reading.humidity = calcHumidity(raw_hum);
    reading.valid = true;
    reading.error = "";
    last_reading_ = reading;

    return reading;
}

float AHT20Sensor::calcTemperature(uint32_t raw) {
    return (float)raw * 200.0f / 1048576.0f - 50.0f;
}

float AHT20Sensor::calcHumidity(uint32_t raw) {
    return (float)raw * 100.0f / 1048576.0f;
}

bool AHT20Sensor::isConnected() {
    if (!initialized_) return false;
    uint8_t status;
    return readStatus(status);
}

} // namespace filament_dryer