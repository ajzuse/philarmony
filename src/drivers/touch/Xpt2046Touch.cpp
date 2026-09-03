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

#include "Xpt2046Touch.hpp"

namespace filament_dryer {

bool Xpt2046Touch::begin(const JsonObject& config) {
    cs_pin_ = config["spi_cs"] | -1;
    irq_pin_ = config["irq_pin"] | -1;
    mosi_pin_ = config["spi_mosi"] | -1;
    miso_pin_ = config["spi_miso"] | -1;
    sclk_pin_ = config["spi_sclk"] | -1;
    setSensitivity(config["sensitivity"] | "medium");

    initialized_ = cs_pin_ >= 0;
#ifndef UNIT_TEST
    initialized_ = initialized_ && mosi_pin_ >= 0 && miso_pin_ >= 0 &&
                   sclk_pin_ >= 0;
#endif
    if (!initialized_) {
        last_error_ = "XPT2046 requires SPI CS/MOSI/MISO/SCLK pins";
        return false;
    }

    pinMode(cs_pin_, OUTPUT);
    digitalWrite(cs_pin_, HIGH);
    if (irq_pin_ >= 0) {
        pinMode(irq_pin_, INPUT_PULLUP);
    }
#ifndef UNIT_TEST
    spi_ = &SPI;
    spi_->begin(sclk_pin_, miso_pin_, mosi_pin_, cs_pin_);
#endif
    last_error_ = "";
    return true;
}

TouchPoint Xpt2046Touch::read() {
    TouchPoint point;
    point.timestamp_ms = millis();
#ifdef UNIT_TEST
    point.pressed = false;
    return point;
#else
    if (!initialized_ || !spi_) {
        return point;
    }
    if (irq_pin_ >= 0 && digitalRead(irq_pin_) != LOW) {
        return point;
    }

    spi_->beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(cs_pin_, LOW);

    const uint16_t z1 = transfer12(0xB1);
    const uint16_t z2 = transfer12(0xC1);
    const uint16_t pressure =
        z1 == 0 ? 0 : static_cast<uint16_t>(z1 + (4095u - z2));
    if (pressure >= pressureThreshold()) {
        uint32_t x_total = 0;
        uint32_t y_total = 0;
        constexpr uint8_t kSamples = 3;
        for (uint8_t i = 0; i < kSamples; ++i) {
            x_total += transfer12(0xD1);
            y_total += transfer12(0x91);
        }
        const uint32_t raw_x = x_total / kSamples;
        const uint32_t raw_y = y_total / kSamples;
        point.x = static_cast<int16_t>(raw_x > 4095u ? 4095u : raw_x);
        point.y = static_cast<int16_t>(raw_y > 4095u ? 4095u : raw_y);
        point.pressed = true;
    }

    // Power down the ADC so PENIRQ remains usable between samples.
    spi_->transfer(0x00);
    spi_->transfer(0x00);
    spi_->transfer(0x00);
    digitalWrite(cs_pin_, HIGH);
    spi_->endTransaction();
    return point;
#endif
}

bool Xpt2046Touch::isConnected() {
    return initialized_;
}

void Xpt2046Touch::setSensitivity(const String& level) {
    sensitivity_ = (level == "low" || level == "high") ? level : String("medium");
}

uint16_t Xpt2046Touch::pressureThreshold() const {
    if (sensitivity_ == "high") return 150;
    if (sensitivity_ == "low") return 700;
    return 400;
}

#ifndef UNIT_TEST
uint16_t Xpt2046Touch::transfer12(uint8_t command) {
    spi_->transfer(command);
    const uint16_t high = spi_->transfer(0x00);
    const uint16_t low = spi_->transfer(0x00);
    return static_cast<uint16_t>(((high << 8) | low) >> 3) & 0x0FFFu;
}
#endif

} // namespace filament_dryer
