/**
 * CustomSensor - Implementation
 * Plugin sensor driver for user-defined sensors
 */
#include "CustomSensor.hpp"

namespace filament_dryer {

CustomSensor::CustomSensor() {}

CustomSensor::~CustomSensor() {}

bool CustomSensor::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    // Check for required callbacks
    if (!init_cb_ || !read_cb_) {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR, 
                         "CustomSensor: Missing required callbacks (init/read)");
        return false;
    }
    
    // Call user init callback
    if (!init_cb_(config)) {
        logMgr.logSystem(LogLevel::ERROR, LogModule::SENSOR, 
                         "CustomSensor: Plugin init callback failed");
        return false;
    }
    
    plugin_name_ = config["name"] | "custom";
    initialized_ = true;
    
    logMgr.logSystem(LogLevel::INFO, LogModule::SENSOR, 
                     "CustomSensor '%s' initialized", plugin_name_.c_str());
    return true;
}

SensorReading CustomSensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    reading.valid = false;
    
    if (!initialized_ || !read_cb_) {
        reading.error = "Not initialized or missing read callback";
        return reading;
    }
    
    try {
        reading = read_cb_();
        last_reading_ = reading;
    } catch (...) {
        reading.error = "Plugin read callback exception";
    }
    
    return reading;
}

String CustomSensor::getType() const {
    return "custom:" + plugin_name_;
}

String CustomSensor::getName() const {
    return "Custom Sensor: " + plugin_name_;
}

bool CustomSensor::isConnected() {
    if (!initialized_ || !connected_cb_) return initialized_;
    return connected_cb_();
}

} // namespace filament_dryer