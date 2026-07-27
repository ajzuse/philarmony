/**
 * DriverRegistry - Implementation
 * Registers all built-in drivers for sensors, actuators, displays, and control algorithms
 */
#include "DriverRegistry.hpp"
#include "ConfigManager.hpp"
#include "drivers/sensors/SHT3xSensor.hpp"
#include "drivers/sensors/DHT22Sensor.hpp"
#include "drivers/sensors/DS18B20Sensor.hpp"
#include "drivers/sensors/NTCSensor.hpp"
#include "drivers/sensors/BME280Sensor.hpp"
#include "drivers/sensors/AHT20Sensor.hpp"
#include "drivers/sensors/CustomSensor.hpp"
#include "drivers/actuators/MosfetActuator.hpp"
#include "drivers/actuators/FanActuator.hpp"
#include "drivers/actuators/SSRActuator.hpp"
#include "drivers/actuators/StepperActuator.hpp"
#include "drivers/actuators/ServoActuator.hpp"
#include "drivers/actuators/GPIOActuator.hpp"
#include "drivers/actuators/SharedMosfetActuator.hpp"
#include "drivers/display/ST7789Display.hpp"
#include "drivers/display/ILI9341Display.hpp"
#include "drivers/display/SSD1306Display.hpp"
#include "drivers/display/SH1106Display.hpp"
#include "drivers/display/ST7735Display.hpp"
#include "drivers/display/GC9A01Display.hpp"
#include "drivers/display/ILI9488Display.hpp"
#include "drivers/display/HD44780Display.hpp"
#include "drivers/display/NextionDisplay.hpp"
#include "drivers/display/DisplayManager.hpp"
#include "control/PIDControl.hpp"
#include "control/BangBangControl.hpp"
#include "control/PWMFeedforwardControl.hpp"

namespace filament_dryer {

void DriverRegistry::registerBuiltins() {
    // ===========================================
    // SENSOR DRIVERS
    // ===========================================
    
    // SHT3x (SHT30, SHT31, SHT35) - I2C Temperature + Humidity
    registerSensor("sht3x", [](const JsonObject& config) -> ISensorDriver* {
        return new SHT3xSensor(config);
    });
    registerSensor("sht30", [](const JsonObject& config) -> ISensorDriver* {
        return new SHT3xSensor(config);
    });
    registerSensor("sht31", [](const JsonObject& config) -> ISensorDriver* {
        return new SHT3xSensor(config);
    });
    registerSensor("sht35", [](const JsonObject& config) -> ISensorDriver* {
        return new SHT3xSensor(config);
    });

    // SHT30 (alias)
    registerSensor("sht30", [](const JsonObject& config) -> ISensorDriver* {
        return new SHT3xSensor(config);
    });

    // DHT22/DHT11/AM2302 - 1-Wire Temperature + Humidity
    registerSensor("dht22", [](const JsonObject& config) -> ISensorDriver* {
        return new DHT22Sensor(config);
    });
    registerSensor("dht11", [](const JsonObject& config) -> ISensorDriver* {
        return new DHT22Sensor(config);
    });
    registerSensor("am2302", [](const JsonObject& config) -> ISensorDriver* {
        return new DHT22Sensor(config);
    });

    // DS18B20/DS18S20 - 1-Wire Temperature Only
    registerSensor("ds18b20", [](const JsonObject& config) -> ISensorDriver* {
        return new DS18B20Sensor(config);
    });
    registerSensor("ds18s20", [](const JsonObject& config) -> ISensorDriver* {
        return new DS18B20Sensor(config);
    });

    // NTC Thermistor - ADC with Beta formula
    registerSensor("ntc", [](const JsonObject& config) -> ISensorDriver* {
        return new NTCSensor(config);
    });
    registerSensor("thermistor", [](const JsonObject& config) -> ISensorDriver* {
        return new NTCSensor(config);
    });

    // BME280/BMP280 - I2C/SPI Temperature + Pressure + Humidity
    registerSensor("bme280", [](const JsonObject& config) -> ISensorDriver* {
        return new BME280Sensor(config);
    });
    registerSensor("bmp280", [](const JsonObject& config) -> ISensorDriver* {
        return new BME280Sensor(config);
    });

    // AHT20/AHT10 - I2C Temperature + Humidity
    registerSensor("aht20", [](const JsonObject& config) -> ISensorDriver* {
        return new AHT20Sensor(config);
    });
    registerSensor("aht10", [](const JsonObject& config) -> ISensorDriver* {
        return new AHT20Sensor(config);
    });

    // Custom sensor plugin (user-provided)
    registerSensor("custom", [](const JsonObject& config) -> ISensorDriver* {
        return new CustomSensor(config);
    });

    // ===========================================
    // ACTUATOR DRIVERS
    // ===========================================

    // MOSFET PWM (AOD4184, IRLB3034, etc.) - Heater
    registerActuator("mosfet_pwm", [](const JsonObject& config) -> IActuatorDriver* {
        return new MosfetActuator(config);
    });

    // SSR (Solid State Relay) - Heater
    registerActuator("ssr", [](const JsonObject& config) -> IActuatorDriver* {
        return new SSRActuator(config);
    });

    // Fan PWM - Exhaust fan with PWM control
    registerActuator("fan_pwm", [](const JsonObject& config) -> IActuatorDriver* {
        return new FanActuator(config);
    });

    // Fan Digital - Simple on/off fan
    registerActuator("fan_digital", [](const JsonObject& config) -> IActuatorDriver* {
        return new FanActuator(config);
    });

    // Shared MOSFET - Heater and fan share same MOSFET output
    registerActuator("shared_mosfet", [](const JsonObject& config) -> IActuatorDriver* {
        return new SharedMosfetActuator(config);
    });

    // SSR (Solid State Relay) - Alternative name
    registerActuator("ssr", [](const JsonObject& config) -> IActuatorDriver* {
        return new SSRActuator(config);
    });

    // Stepper Motor - For custom actuators
    registerActuator("stepper", [](const JsonObject& config) -> IActuatorDriver* {
        return new StepperActuator(config);
    });

    // Servo Motor - For custom actuators
    registerActuator("servo", [](const JsonObject& config) -> IActuatorDriver* {
        return new ServoActuator(config);
    });

    // Generic GPIO - Simple on/off
    registerActuator("gpio", [](const JsonObject& config) -> IActuatorDriver* {
        return new GPIOActuator(config);
    });

    // Shared MOSFET - Heater and fan share same output
    registerActuator("shared_mosfet", [](const JsonObject& config) -> IActuatorDriver* {
        return new SharedMosfetActuator(config);
    });

    // ===========================================
    // DISPLAY DRIVERS
    // ===========================================

    // SSD1306 - I2C/SPI OLED 128x64/32
    registerDisplay("ssd1306", [](const JsonObject& config) -> IDisplayDriver* {
        return new SSD1306Display(config);
    });

    // SH1106 - I2C/SPI OLED 128x64
    registerDisplay("sh1106", [](const JsonObject& config) -> IDisplayDriver* {
        return new SH1106Display(config);
    });

    // ST7789 - SPI TFT 135x240, 240x240, 170x320 (LilyGo T-Display, TTGO, T-QT)
    registerDisplay("st7789", [](const JsonObject& config) -> IDisplayDriver* {
        return new ST7789Display(config);
    });

    // ILI9341 - SPI TFT 240x320 (ESP32-2432S028 CYD)
    registerDisplay("ili9341", [](const JsonObject& config) -> IDisplayDriver* {
        return new ILI9341Display(config);
    });

    // ST7735 - SPI TFT 128x128, 128x160
    registerDisplay("st7735", [](const JsonObject& config) -> IDisplayDriver* {
        return new ST7735Display(config);
    });

    // GC9A01 - SPI Round TFT 240x240
    registerDisplay("gc9a01", [](const JsonObject& config) -> IDisplayDriver* {
        return new GC9A01Display(config);
    });

    // ILI9488 - SPI/Parallel 320x480
    registerDisplay("ili9488", [](const JsonObject& config) -> IDisplayDriver* {
        return new ILI9488Display(config);
    });

    // HD44780 - I2C Character LCD 16x2, 20x4
    registerDisplay("hd44780", [](const JsonObject& config) -> IDisplayDriver* {
        return new HD44780Display(config);
    });

    // Nextion - UART Serial HMI
    registerDisplay("nextion", [](const JsonObject& config) -> IDisplayDriver* {
        return new NextionDisplay(config);
    });

    // Auto-detect display
    registerDisplay("auto", [](const JsonObject& config) -> IDisplayDriver* {
        // Will try ST7789, ILI9341, SSD1306 in order
        return new DisplayManager(config);
    });

    // ===========================================
    // CONTROL ALGORITHMS
    // ===========================================

    // PID Control - Classic PID with anti-windup
    registerControl("pid", [](const JsonObject& config) -> IControlAlgorithm* {
        return new PIDControl(config);
    });

    // Bang-Bang (Hysteresis) - Simple on/off with hysteresis
    registerControl("bang_bang", [](const JsonObject& config) -> IControlAlgorithm* {
        return new BangBangControl(config);
    });

    // PWM Feedforward - Base PWM + temperature coefficient
    registerControl("pwm_feedforward", [](const JsonObject& config) -> IControlAlgorithm* {
        return new PWMFeedforwardControl(config);
    });

    // Custom algorithm plugin
    registerControl("custom", [](const JsonObject& config) -> IControlAlgorithm* {
        // Will be handled by plugin system
        return nullptr;
    });
}

} // namespace filament_dryer