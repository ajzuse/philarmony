/*
 * Philarmony Filament Dryer — Desktop Installer / Shared Core
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

/// Embedded copy of specs/002-esp32-desktop-installer/contracts/installer-profile.schema.json
const String kInstallerProfileSchemaJson = r'''
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://philarmony.local/schemas/installer-profile-v1.json",
  "title": "Philarmony ESP32 Installer Profile",
  "type": "object",
  "required": [
    "profile_version",
    "device_model",
    "flash_size_mb",
    "sensors",
    "pin_mapping",
    "display",
    "filament_profiles",
    "wifi",
    "firmware_version"
  ],
  "properties": {
    "profile_version": { "type": "integer", "const": 1 },
    "device_model": {
      "type": "string",
      "enum": ["ESP32", "ESP32-S2", "ESP32-S3", "ESP32-C3"]
    },
    "flash_size_mb": { "type": "integer", "enum": [4, 8, 16] },
    "sensors": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["role", "sensor"],
        "properties": {
          "role": {
            "type": "string",
            "enum": ["temperature", "humidity", "pressure", "extra"]
          },
          "sensor": {
            "type": "string",
            "enum": [
              "none",
              "dht22",
              "ds18b20",
              "bme280",
              "sht3x",
              "sht31",
              "ntc",
              "aht20"
            ]
          },
          "gpio_pin": { "type": ["integer", "null"], "minimum": 0, "maximum": 48 },
          "i2c_address": { "type": ["integer", "null"], "minimum": 1, "maximum": 127 },
          "parameters": { "type": "object", "additionalProperties": true }
        },
        "additionalProperties": false
      }
    },
    "pin_mapping": {
      "type": "object",
      "required": ["heater_pwm"],
      "properties": {
        "heater_pwm": { "type": "integer", "minimum": 0, "maximum": 48 },
        "exhaust_fan_pwm": { "type": ["integer", "null"] },
        "exhaust_fan_digital": { "type": ["integer", "null"] },
        "i2c_sda": { "type": ["integer", "null"] },
        "i2c_scl": { "type": ["integer", "null"] },
        "spi_mosi": { "type": ["integer", "null"] },
        "spi_miso": { "type": ["integer", "null"] },
        "spi_sck": { "type": ["integer", "null"] },
        "spi_cs": { "type": ["integer", "null"] },
        "spi_dc": { "type": ["integer", "null"] },
        "spi_reset": { "type": ["integer", "null"] }
      },
      "additionalProperties": false
    },
    "display": {
      "type": "object",
      "required": ["enabled", "driver", "width", "height", "fields"],
      "properties": {
        "enabled": { "type": "boolean" },
        "driver": {
          "type": "string",
          "enum": [
            "none",
            "auto",
            "ssd1306",
            "sh1106",
            "st7789",
            "ili9341",
            "st7735",
            "gc9a01",
            "ili9488",
            "hd44780",
            "nextion"
          ]
        },
        "width": { "type": "integer", "minimum": 1 },
        "height": { "type": "integer", "minimum": 1 },
        "i2c_address": { "type": ["integer", "null"] },
        "fields": {
          "type": "array",
          "items": { "type": "string" }
        },
        "refresh_rate_hz": { "type": "integer", "minimum": 1, "maximum": 5, "default": 1 }
      },
      "additionalProperties": false
    },
    "filament_profiles": {
      "type": "array",
      "maxItems": 25,
      "items": {
        "type": "object",
        "required": [
          "id",
          "name_pt",
          "name_en",
          "target_temp_c",
          "default_duration_min",
          "target_humidity_pct",
          "is_builtin"
        ],
        "properties": {
          "id": { "type": "string", "minLength": 1 },
          "name_pt": { "type": "string" },
          "name_en": { "type": "string" },
          "target_temp_c": { "type": "number", "minimum": 30, "maximum": 80 },
          "default_duration_min": { "type": "integer", "minimum": 1, "maximum": 1440 },
          "target_humidity_pct": { "type": "number", "minimum": 5, "maximum": 50 },
          "is_builtin": { "type": "boolean" }
        },
        "additionalProperties": false
      }
    },
    "wifi": {
      "type": "object",
      "required": ["ssid"],
      "properties": {
        "ssid": { "type": "string", "minLength": 1 },
        "password": {
          "type": "string",
          "description": "Omit or use placeholder when exporting profiles"
        },
        "static_ip": {
          "type": ["object", "null"],
          "properties": {
            "ip": { "type": "string" },
            "gateway": { "type": "string" },
            "netmask": { "type": "string" },
            "dns": { "type": "string" }
          }
        }
      },
      "additionalProperties": false
    },
    "firmware_version": { "type": "string", "minLength": 1 },
    "created_at": { "type": "string", "format": "date-time" },
    "updated_at": { "type": "string", "format": "date-time" }
  },
  "additionalProperties": false
}

''';
