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

import '../models/device_profile.dart';

class PinValidationResult {
  PinValidationResult({required this.ok, List<String>? errors})
      : errors = errors ?? [];

  final bool ok;
  final List<String> errors;
}

/// Validates GPIO assignments before flash (failsafe).
class PinValidator {
  /// Classic ESP32 input-only GPIOs (cannot drive heater/fan outputs).
  static const inputOnlyEsp32 = {34, 35, 36, 37, 38, 39};

  /// Common strapping pins that should be warned/blocked for outputs.
  static const strappingEsp32 = {0, 2, 12, 15};

  PinValidationResult validate(DeviceProfile profile) {
    final errors = <String>[];
    final pins = profile.pinMapping.assignedPins().toList();
    final seen = <int, String>{};

    for (final e in pins) {
      final pin = e.value;
      final name = e.key;
      if (seen.containsKey(pin)) {
        errors.add('Duplicate GPIO $pin: ${seen[pin]} and $name');
      } else {
        seen[pin] = name;
      }

      if (profile.deviceModel == 'ESP32' &&
          inputOnlyEsp32.contains(pin) &&
          _isOutput(name)) {
        errors.add('GPIO $pin is input-only on ESP32; cannot assign $name');
      }

      if (strappingEsp32.contains(pin) && _isOutput(name)) {
        errors.add('GPIO $pin is a strapping pin; unsafe for $name');
      }
    }

    for (final s in profile.sensors) {
      if (s.sensor == 'none') continue;
      if (s.gpioPin != null) {
        if (seen.containsKey(s.gpioPin) &&
            seen[s.gpioPin] != 'sensor:${s.role}') {
          // allow shared DHT temp+humidity on same pin
          final existing = seen[s.gpioPin]!;
          if (!existing.startsWith('sensor:')) {
            errors.add('Sensor ${s.role} GPIO ${s.gpioPin} conflicts with $existing');
          }
        } else if (s.gpioPin != null) {
          seen[s.gpioPin!] = 'sensor:${s.role}';
        }
      }
    }

    final p = profile.pinMapping;
    final needsI2c = profile.sensors.any((s) =>
            {'bme280', 'sht3x', 'sht31', 'aht20'}.contains(s.sensor)) ||
        (profile.display.enabled &&
            {'ssd1306', 'sh1106'}.contains(profile.display.driver));
    if (needsI2c && (p.i2cSda == null || p.i2cScl == null)) {
      errors.add('I2C SDA/SCL required for selected sensors/display');
    }

    if (p.exhaustFanPwm != null && p.exhaustFanDigital != null) {
      errors.add('Choose either exhaust_fan_pwm or exhaust_fan_digital, not both');
    }

    return PinValidationResult(ok: errors.isEmpty, errors: errors);
  }

  bool _isOutput(String name) =>
      name.contains('pwm') ||
      name.contains('digital') ||
      name.startsWith('spi_') ||
      name.startsWith('i2c_');
}
