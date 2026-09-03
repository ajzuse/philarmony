/*
 * Philarmony Filament Dryer ESP32 Firmware
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Stmpe610Touch.hpp"

namespace filament_dryer {

namespace {
constexpr uint8_t kRegChipId = 0x00;
constexpr uint8_t kRegSysCtrl1 = 0x03;
constexpr uint8_t kRegSysCtrl2 = 0x04;
constexpr uint8_t kRegIntCtrl = 0x09;
constexpr uint8_t kRegIntEn = 0x0A;
constexpr uint8_t kRegIntSta = 0x0B;
constexpr uint8_t kRegAdcCtrl1 = 0x20;
constexpr uint8_t kRegAdcCtrl2 = 0x21;
constexpr uint8_t kRegTscCtrl = 0x40;
constexpr uint8_t kRegTscCfg = 0x41;
constexpr uint8_t kRegFifoTh = 0x4A;
constexpr uint8_t kRegFifoSta = 0x4B;
constexpr uint8_t kRegFifoSize = 0x4C;
constexpr uint8_t kRegTscDataX = 0x4D;
constexpr uint8_t kRegTscDataY = 0x4F;
constexpr uint8_t kRegTscDataZ = 0x51;
constexpr uint8_t kRegTscFraq = 0x56;
constexpr uint8_t kRegTscIDrive = 0x58;
constexpr uint8_t kSpiRead = 0x80;
}  // namespace

bool Stmpe610Touch::begin(const JsonObject& config) {
    cs_pin_ = config["spi_cs"] | -1;
    irq_pin_ = config["irq_pin"] | -1;
    mosi_pin_ = config["spi_mosi"] | -1;
    miso_pin_ = config["spi_miso"] | -1;
    sclk_pin_ = config["spi_sclk"] | -1;
    i2c_address_ = config["i2c_address"] | 0x41;
    setSensitivity(config["sensitivity"] | "medium");
    const String bus = config["bus"] | "i2c";
    use_spi_ = bus == "spi" && cs_pin_ >= 0 && mosi_pin_ >= 0 &&
               miso_pin_ >= 0 && sclk_pin_ >= 0;

#ifndef UNIT_TEST
    if (use_spi_) {
        pinMode(cs_pin_, OUTPUT);
        digitalWrite(cs_pin_, HIGH);
        spi_ = &SPI;
        spi_->begin(sclk_pin_, miso_pin_, mosi_pin_, cs_pin_);
    } else {
        const int8_t sda_pin = config["sda_pin"] | 21;
        const int8_t scl_pin = config["scl_pin"] | 22;
        const uint8_t i2c_bus = config["i2c_bus"] | 0;
#if defined(ESP32)
        wire_ = i2c_bus == 0 ? &Wire : &Wire1;
#else
        (void)i2c_bus;
        wire_ = &Wire;
#endif
        wire_->begin(sda_pin, scl_pin);
        wire_->setClock(100000);
    }
    if (irq_pin_ >= 0) {
        pinMode(irq_pin_, INPUT_PULLUP);
    }

    writeRegister(kRegSysCtrl1, 0x02);
    delay(10);
    writeRegister(kRegSysCtrl2, 0x04);
    writeRegister(kRegIntEn, 0x01);
    writeRegister(kRegAdcCtrl1, 0x49);
    writeRegister(kRegAdcCtrl2, 0x01);
    writeRegister(kRegTscCfg, 0x9A);
    writeRegister(kRegFifoTh, 0x01);
    writeRegister(kRegFifoSta, 0x01);
    writeRegister(kRegTscFraq, 0x01);
    writeRegister(kRegTscIDrive, 0x01);
    writeRegister(kRegIntSta, 0xFF);
    writeRegister(kRegIntCtrl, 0x01);
    writeRegister(kRegTscCtrl, 0x03);

    initialized_ = identify();
    if (!initialized_) {
        last_error_ = "STMPE610 not found";
        return false;
    }
    last_error_ = "";
    return true;
#else
    initialized_ = true;
    (void)config;
    return true;
#endif
}

TouchPoint Stmpe610Touch::read() {
    TouchPoint point;
    point.timestamp_ms = millis();
    if (!initialized_) {
        return point;
    }
#ifdef UNIT_TEST
    return point;
#else
    uint8_t fifo = 0;
    if (!readRegisters(kRegFifoSize, &fifo, 1) || fifo == 0) {
        return point;
    }
    uint8_t xyz[5] = {};
    if (!readRegisters(kRegTscDataX, xyz, sizeof(xyz))) {
        return point;
    }
    const uint16_t x = static_cast<uint16_t>((xyz[0] << 4) | (xyz[1] >> 4));
    const uint16_t y =
        static_cast<uint16_t>(((xyz[1] & 0x0F) << 8) | xyz[2]);
    const uint16_t z = static_cast<uint16_t>((xyz[3] << 8) | xyz[4]);
    writeRegister(kRegIntSta, 0xFF);
    writeRegister(kRegFifoSta, 0x01);
    if (z < pressureThreshold()) {
        return point;
    }
    point.pressed = true;
    point.x = static_cast<int16_t>(x);
    point.y = static_cast<int16_t>(y);
    return point;
#endif
}

bool Stmpe610Touch::isConnected() {
    return initialized_ && identify();
}

void Stmpe610Touch::setSensitivity(const String& level) {
    sensitivity_ =
        (level == "low" || level == "high") ? level : String("medium");
}

uint16_t Stmpe610Touch::pressureThreshold() const {
    if (sensitivity_ == "high") return 50;
    if (sensitivity_ == "low") return 200;
    return 100;
}

bool Stmpe610Touch::identify() {
#ifdef UNIT_TEST
    return initialized_;
#else
    uint8_t id[2] = {};
    if (!readRegisters(kRegChipId, id, 2)) {
        return false;
    }
    return id[0] == 0x08 && id[1] == 0x11;
#endif
}

bool Stmpe610Touch::writeRegister(uint8_t reg, uint8_t value) {
#ifdef UNIT_TEST
    (void)reg;
    (void)value;
    return true;
#else
    if (use_spi_ && spi_) {
        spi_->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(cs_pin_, LOW);
        spi_->transfer(reg);
        spi_->transfer(value);
        digitalWrite(cs_pin_, HIGH);
        spi_->endTransaction();
        return true;
    }
    if (!wire_) {
        return false;
    }
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    wire_->write(value);
    return wire_->endTransmission() == 0;
#endif
}

bool Stmpe610Touch::readRegisters(uint8_t reg, uint8_t* data, size_t length) {
#ifdef UNIT_TEST
    (void)reg;
    (void)data;
    (void)length;
    return false;
#else
    if (data == nullptr || length == 0) {
        return false;
    }
    if (use_spi_ && spi_) {
        spi_->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(cs_pin_, LOW);
        spi_->transfer(reg | kSpiRead);
        for (size_t i = 0; i < length; ++i) {
            data[i] = spi_->transfer(0x00);
        }
        digitalWrite(cs_pin_, HIGH);
        spi_->endTransaction();
        return true;
    }
    if (!wire_) {
        return false;
    }
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    if (wire_->endTransmission(false) != 0) {
        return false;
    }
    const size_t got = wire_->requestFrom(i2c_address_, length);
    size_t received = 0;
    while (wire_->available() && received < length) {
        data[received++] = static_cast<uint8_t>(wire_->read());
    }
    return got == length && received == length;
#endif
}

}  // namespace filament_dryer
