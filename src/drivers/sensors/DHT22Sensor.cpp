/**
 * DHT22Sensor - Implementation
 */
#include "DHT22Sensor.hpp"

namespace filament_dryer {

DHT22Sensor::DHT22Sensor() {}

DHT22Sensor::~DHT22Sensor() {}

bool DHT22Sensor::begin(const JsonObject& config) {
    gpio_pin_ = config["gpio_pin"] | 4;
    
    pinMode(gpio_pin_, INPUT_PULLUP);
    delay(1000); // DHT22 needs 1s after power-up
    
    // Test read
    last_reading_ = read();
    initialized_ = last_reading_.valid;
    
    if (initialized_) {
        Serial.printf("[DHT22Sensor] Initialized on GPIO %d\n", gpio_pin_);
    } else {
        Serial.printf("[DHT22Sensor] WARNING: Initial read failed on GPIO %d\n", gpio_pin_);
    }
    
    return initialized_;
}

SensorReading DHT22Sensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    
    // Respect minimum read interval
    if (millis() - last_read_time_ < MIN_READ_INTERVAL_MS) {
        reading = last_reading_;
        reading.timestamp = millis();
        return reading;
    }
    
    uint8_t data[5] = {0};
    
    if (readData(data)) {
        // Verify checksum
        uint8_t checksum = data[0] + data[1] + data[2] + data[3];
        if (checksum == data[4]) {
            int16_t humidity_raw = (data[0] << 8) | data[1];
            int16_t temp_raw = (data[2] << 8) | data[3];
            
            reading.temperature = calculateTemperature(temp_raw);
            reading.humidity = calculateHumidity(humidity_raw);
            reading.valid = true;
            last_reading_ = reading;
            last_read_time_ = millis();
            clearError();
        } else {
            setError("Checksum mismatch");
            reading.valid = false;
            reading.error_message = "Checksum error";
        }
    } else {
        setError("Timeout or communication error");
        reading.valid = false;
        reading.error_message = "Communication timeout";
    }
    
    return reading;
}

bool DHT22Sensor::readData(uint8_t* data) {
    // Start signal: pull low for >1ms, then high for 40us
    pinMode(gpio_pin_, OUTPUT);
    digitalWrite(gpio_pin_, LOW);
    delay(20); // >1ms (18ms min)
    digitalWrite(gpio_pin_, HIGH);
    delayMicroseconds(40);
    
    // Switch to input with pullup
    pinMode(gpio_pin_, INPUT_PULLUP);
    
    // Wait for sensor response (80us low, 80us high)
    uint32_t timeout = micros() + 100;
    while (digitalRead(gpio_pin_) == HIGH) {
        if (micros() > timeout) return false;
    }
    
    timeout = micros() + 100;
    while (digitalRead(gpio_pin_) == LOW) {
        if (micros() > timeout) return false;
    }
    
    timeout = micros() + 100;
    while (digitalRead(gpio_pin_) == HIGH) {
        if (micros() > timeout) return false;
    }
    
    // Read 40 bits (5 bytes)
    for (int byte_idx = 0; byte_idx < 5; byte_idx++) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; bit++) {
            // Wait for low to high transition
            timeout = micros() + 100;
            while (digitalRead(gpio_pin_) == LOW) {
                if (micros() > timeout) return false;
            }
            
            // Measure high pulse width
            uint32_t t = micros();
            timeout = micros() + 100;
            while (digitalRead(gpio_pin_) == HIGH) {
                if (micros() > timeout) return false;
            }
            
            // Pulse > 50us = 1, < 50us = 0
            if (micros() - t > 50) {
                byte |= (1 << (7 - bit));
            }
        }
        data[byte_idx] = byte;
    }
    
    return true;
}

float DHT22Sensor::calculateTemperature(int16_t raw) {
    return raw / 10.0f;
}

float DHT22Sensor::calculateHumidity(int16_t raw) {
    return raw / 10.0f;
}

bool DHT22Sensor::isConnected() {
    return initialized_ && last_reading_.valid;
}

} // namespace filament_dryer