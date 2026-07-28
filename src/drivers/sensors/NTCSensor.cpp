/**
 * NTCSensor - Implementation
 * NTC Thermistor ADC-based temperature sensor with Steinhart-Hart equation
 */
#include "NTCSensor.hpp"
#include <math.h>

namespace filament_dryer {

NTCSensor::NTCSensor() {}

NTCSensor::~NTCSensor() {}

bool NTCSensor::begin(const JsonObject& config) {
    if (initialized_) return true;
    
    gpio_pin_ = config["gpio_pin"] | 34;
    beta_coefficient_ = config["beta"] | 3950.0f;
    series_resistor_ = config["series_resistor"] | 10000.0f;
    nominal_resistance_ = config["nominal_resistance"] | 10000.0f;
    nominal_temperature_ = config["nominal_temperature"] | 25.0f;
    adc_attenuation_ = config["adc_attenuation"] | 11;
    adc_width_ = config["adc_width"] | 12;

    // Configure ADC
    pinMode(gpio_pin_, INPUT);
    analogReadResolution(adc_width_);
    analogSetAttenuation((adc_attenuation_t)adc_attenuation_);

    initialized_ = true;
    

    return true;
}

SensorReading NTCSensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    reading.valid = false;
    reading.temperature = NAN;
    reading.humidity = NAN;
    reading.pressure = NAN;

    if (!initialized_) {
        reading.error_message = "Not initialized";
        return reading;
    }

    // Read ADC value
    uint32_t adc_raw = analogRead(gpio_pin_);
    
    // Convert to resistance
    uint32_t max_adc = (1 << adc_width_) - 1;  // 4095 for 12-bit
    if (adc_raw >= max_adc) {
        reading.error_message = "ADC saturated (open circuit?)";
        return reading;
    }
    if (adc_raw == 0) {
        reading.error_message = "ADC zero (short circuit?)";
        return reading;
    }

    // Calculate thermistor resistance from voltage divider
    // Vout = Vin * (R_ntc / (R_series + R_ntc))
    // ADC = max_adc * Vout / Vref
    // R_ntc = R_series * ADC / (max_adc - ADC)
    float adc_ratio = (float)adc_raw / (float)max_adc;
    float r_ntc = series_resistor_ * adc_ratio / (1.0f - adc_ratio);
    
    // Steinhart-Hart equation (simplified Beta formula)
    // 1/T = 1/T0 + (1/Beta) * ln(R/R0)
    // T = 1 / (1/T0 + (1/Beta) * ln(R/R0))
    float t0 = nominal_temperature_ + 273.15f;  // Convert to Kelvin
    float inv_T = (1.0f / t0) + (1.0f / beta_coefficient_) * log(r_ntc / nominal_resistance_);
    float temp_kelvin = 1.0f / inv_T;
    float temp_celsius = temp_kelvin - 273.15f;

    // Sanity check
    if (temp_celsius < -50.0f || temp_celsius > 200.0f) {
        reading.error_message = "Temperature out of valid range";
        return reading;
    }

    reading.temperature = temp_celsius;
    reading.humidity = NAN;
    reading.pressure = NAN;
    reading.valid = true;
    reading.error_message = "";
    last_reading_ = reading;

    return reading;
}

bool NTCSensor::isConnected() {
    if (!initialized_) return false;
    SensorReading r = read();
    return r.valid;
}

} // namespace filament_dryer