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

/**
 * LvglPort - Optional LVGL display flush and touch input adapter
 */
#pragma once

#include <Arduino.h>

#include "../drivers/interfaces/IDriverInterfaces.hpp"

namespace filament_dryer {

class TouchManager;

class LvglPort {
public:
    using FlushCallback = void (*)(int16_t x1, int16_t y1, int16_t x2,
                                   int16_t y2, const uint16_t* pixels,
                                   void* context);

    LvglPort() = default;
    ~LvglPort();

    bool init(TouchManager& touch_manager, uint16_t width, uint16_t height,
              FlushCallback flush_callback = nullptr, void* context = nullptr);
    void deinit();
    void tick();

    void flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
               const uint16_t* pixels);
    TouchPoint readInput();
    bool isInitialized() const { return initialized_; }
    uint32_t lastTouchMs() const { return last_touch_ms_; }
    TouchPoint lastTouchPoint() const { return last_touch_point_; }

private:
    struct Impl;
    Impl* impl_ = nullptr;
    TouchManager* touch_manager_ = nullptr;
    FlushCallback flush_callback_ = nullptr;
    void* callback_context_ = nullptr;
    bool initialized_ = false;
    uint32_t last_touch_ms_ = 0;
    TouchPoint last_touch_point_;
};

} // namespace filament_dryer
