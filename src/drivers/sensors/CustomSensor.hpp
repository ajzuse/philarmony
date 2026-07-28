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
 * CustomSensor - Plugin sensor driver template
 * Allows users to implement custom sensors without modifying core firmware
 */
#pragma once

#include "../interfaces/IDriverInterfaces.hpp"
#include <functional>

namespace filament_dryer {

class CustomSensor : public ISensorDriver {
public:
    using ReadCallback = std::function<SensorReading()>;
    using InitCallback = std::function<bool(const JsonObject&)>;
    using ConnectedCallback = std::function<bool()>;

    CustomSensor();
    ~CustomSensor() override;
    
    bool begin(const JsonObject& config) override;
    SensorReading read() override;
    String getType() const override;
    String getName() const override;
    bool isConnected() override;

    // Setters for plugin callbacks
    void setInitCallback(InitCallback cb) { init_cb_ = std::move(cb); }
    void setReadCallback(ReadCallback cb) { read_cb_ = std::move(cb); }
    void setConnectedCallback(ConnectedCallback cb) { connected_cb_ = std::move(cb); }

private:
    InitCallback init_cb_;
    ReadCallback read_cb_;
    ConnectedCallback connected_cb_;
    bool initialized_ = false;
    String plugin_name_ = "custom";
    SensorReading last_reading_;
};

} // namespace filament_dryer