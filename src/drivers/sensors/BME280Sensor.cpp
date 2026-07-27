/**
 * BME280/BMP280 Sensor Driver - Implementation
 * I2C Temperature + Pressure + Humidity sensor driver
 */
#include "BME280Sensor.hpp"

namespace filament_dryer {

BME280Sensor::BME280Sensor() {}

BME280Sensor::~BME280Sensor() {}

bool BME280Sensor::begin(const JsonObject& config) {
    if (initialized_) return true;

    i2c_address_ = config["i2c_address"] | 0x76;
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

    // Read chip ID
    uint8_t id;
    if (!readRegisters(REG_ID, &id, 1)) {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR, "BME280: Failed to read ID at 0x%02X", i2c_address_);
        return false;
    }

    // BME280 = 0x60, BMP280 = 0x58
    if (id == 0x60) {
        is_bmp280_ = false;
        logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, "BME280 detected at 0x%02X", i2c_address_);
    } else if (id == 0x58) {
        is_bmp280_ = true;
        logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, "BMP280 detected at 0x%02X (no humidity)", i2c_address_);
    } else {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR, "Unknown sensor ID: 0x%02X", id);
        return false;
    }

    // Soft reset
    writeRegister(0xE0, 0xB6);
    delay(10);

    // Read calibration data
    if (!readCalibration()) {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR, "BME280: Failed to read calibration data");
        return false;
    }

    // Configure sensor
    // Humidity oversampling x1 (only for BME280)
    if (!is_bmp280_) {
        writeRegister(REG_CTRL_HUM, 0x01);
    }

    // Normal mode, temp/press oversampling x1
    writeRegister(REG_CTRL_MEAS, 0x27);  // osrs_t=1, osrs_p=1, mode=normal
    
    // Config: t_sb=0.5ms, filter=off
    writeRegister(REG_CONFIG, 0x00);

    initialized_ = true;
    logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, 
                     "%s initialized at 0x%02X (I2C bus %d)", 
                     is_bmp280_ ? "BMP280" : "BME280", i2c_address_, i2c_bus_);

    return true;
}

SensorReading BME280Sensor::read() {
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

    // Read raw data (8 bytes: press[3], temp[3], hum[2])
    uint8_t data[8];
    if (!readRegisters(REG_PRESS_MSB, data, 8)) {
        reading.error = "I2C read failed";
        return reading;
    }

    // Parse pressure (20-bit)
    int32_t adc_P = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4);
    
    // Parse temperature (20-bit)
    int32_t adc_T = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | (data[5] >> 4);
    
    // Parse humidity (16-bit) - only for BME280
    int32_t adc_H = 0;
    if (!is_bmp280_) {
        adc_H = ((uint32_t)data[6] << 8) | data[7];
    }

    // Compensate
    reading.temperature = compensateTemperature(adc_T);
    reading.pressure = compensatePressure(adc_P) / 100.0f;  // Convert to hPa
    
    if (!is_bmp280_) {
        reading.humidity = compensateHumidity(adc_H);
    } else {
        reading.humidity = NAN;
    }

    reading.valid = true;
    reading.error = "";
    last_reading_ = reading;
    
    return reading;
}

bool BME280Sensor::isConnected() {
    if (!initialized_) return false;
    uint8_t id;
    return readRegisters(REG_ID, &id, 1) && (id == 0x60 || id == 0x58);
}

bool BME280Sensor::writeRegister(uint8_t reg, uint8_t value) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    wire_->write(value);
    return wire_->endTransmission() == 0;
}

bool BME280Sensor::readRegisters(uint8_t reg, uint8_t* data, size_t len) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    if (wire_->endTransmission(false) != 0) return false;
    
    size_t received = wire_->requestFrom(i2c_address_, len);
    if (received != len) return false;
    
    for (size_t i = 0; i < len; i++) {
        data[i] = wire_->read();
    }
    return true;
}

bool BME280Sensor::readCalibration() {
    // Read 24 bytes from 0x88
    uint8_t calib[24];
    if (!readRegisters(REG_CALIB_START, calib, 24)) return false;

    calib_.dig_T1 = (uint16_t)(calib[1] << 8) | calib[0];
    calib_.dig_T2 = (int16_t)((calib[3] << 8) | calib[2]);
    calib_.dig_T3 = (int16_t)((calib[5] << 8) | calib[4]);
    calib_.dig_P1 = (uint16_t)(calib[7] << 8) | calib[6];
    calib_.dig_P2 = (int16_t)((calib[9] << 8) | calib[8]);
    calib_.dig_P3 = (int16_t)((calib[11] << 8) | calib[10]);
    calib_.dig_P4 = (int16_t)((calib[13] << 8) | calib[12]);
    calib_.dig_P5 = (int16_t)((calib[15] << 8) | calib[14]);
    calib_.dig_P6 = (int16_t)((calib[17] << 8) | calib[16]);
    calib_.dig_P7 = (int16_t)((calib[19] << 8) | calib[18]);
    calib_.dig_P8 = (int16_t)((calib[21] << 8) | calib[20]);
    calib_.dig_P9 = (int16_t)((calib[23] << 8) | calib[22]);

    // Humidity calibration (only for BME280)
    if (!is_bmp280_) {
        uint8_t h_calib[7];
        if (!readRegisters(REG_HUM_CALIB, h_calib, 7)) return false;
        
        calib_.dig_H1 = h_calib[0];
        calib_.dig_H2 = (int16_t)((h_calib[2] << 8) | h_calib[1]);
        calib_.dig_H3 = h_calib[3];
        calib_.dig_H4 = (int16_t)((h_calib[4] << 4) | (h_calib[5] & 0x0F));
        calib_.dig_H5 = (int16_t)((h_calib[6] << 4) | (h_calib[5] >> 4));
        calib_.dig_H6 = (int8_t)h_calib[6];  // Note: h_calib[6] reused, check datasheet
    }

    return true;
}

float BME280Sensor::compensateTemperature(int32_t adc_T) {
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)calib_.dig_T1 << 1))) * ((int32_t)calib_.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib_.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib_.dig_T1))) >> 12) * ((int32_t)calib_.dig_T3)) >> 14;
    t_fine_ = var1 + var2;
    float T = (t_fine_ * 5 + 128) >> 8;
    return T / 100.0f;
}

float BME280Sensor::compensatePressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine_) - 128000;
    var2 = var1 * var1 * (int64_t)calib_.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib_.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib_.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_.dig_P3) >> 8) + ((var1 * (int64_t)calib_.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_.dig_P1) >> 33;
    if (var1 == 0) return 0;
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_.dig_P7) << 4);
    return (float)p / 256.0f;  // Return in Pa
}

float BME280Sensor::compensateHumidity(int32_t adc_H) {
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine_ - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib_.dig_H4) << 20) - (((int32_t)calib_.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)calib_.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)calib_.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)calib_.dig_H2) + 8192) >> 14);
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)calib_.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    float h = (float)(v_x1_u32r >> 12) / 1024.0f;
    return h;
}

} // namespace filament_dryer