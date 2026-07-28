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