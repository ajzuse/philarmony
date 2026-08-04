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

/// Model-specific GPIO constraints (FR-004).
class EspGpioMap {
  const EspGpioMap({
    required this.model,
    required this.usableGpios,
    required this.strapping,
    required this.inputOnly,
    required this.pwmCapable,
    required this.adcCapable,
    required this.flashReserved,
    required this.psramReserved,
  });

  final String model;
  final List<int> usableGpios;
  final Set<int> strapping;
  final Set<int> inputOnly;
  final Set<int> pwmCapable;
  final Set<int> adcCapable;
  final Set<int> flashReserved;
  final Set<int> psramReserved;

  Set<int> get reserved => {...flashReserved, ...psramReserved, ...strapping};

  static EspGpioMap forModel(String model) {
    switch (model) {
      case 'ESP32-S2':
        return EspGpioMap(
          model: model,
          usableGpios: List.generate(47, (i) => i)
              .where((g) => ![22, 23, 24, 25].contains(g))
              .toList(),
          strapping: {0, 45, 46},
          inputOnly: const {},
          pwmCapable: {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21},
          adcCapable: {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14},
          flashReserved: {26, 27, 28, 29, 30, 31, 32},
          psramReserved: const {},
        );
      case 'ESP32-S3':
        return EspGpioMap(
          model: model,
          usableGpios: [
            ...List.generate(21, (i) => i),
            ...List.generate(18, (i) => i + 26),
          ],
          strapping: {0, 3, 45, 46},
          inputOnly: const {},
          pwmCapable: {
            ...List.generate(21, (i) => i),
            ...List.generate(18, (i) => i + 26),
          },
          adcCapable: {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14},
          flashReserved: {22, 23, 24, 25},
          psramReserved: {35, 36, 37},
        );
      case 'ESP32-C3':
        return EspGpioMap(
          model: model,
          usableGpios: List.generate(22, (i) => i),
          strapping: {2, 8, 9},
          inputOnly: const {},
          pwmCapable: {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
          adcCapable: {0, 1, 2, 3, 4},
          flashReserved: {12, 13, 14, 15, 16, 17},
          psramReserved: const {},
        );
      case 'ESP32':
      default:
        return EspGpioMap(
          model: 'ESP32',
          usableGpios: [
            0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23,
            25, 26, 27, 32, 33, 34, 35, 36, 39,
          ],
          strapping: {0, 2, 12, 15},
          inputOnly: {34, 35, 36, 37, 38, 39},
          pwmCapable: {
            0, 2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33,
          },
          adcCapable: {32, 33, 34, 35, 36, 39},
          flashReserved: {6, 7, 8, 9, 10, 11},
          psramReserved: {16, 17},
        );
    }
  }

  static const supportedModels = ['ESP32', 'ESP32-S2', 'ESP32-S3', 'ESP32-C3'];
}

/// Validates GPIO assignments before flash (failsafe).
class PinValidator {
  /// Classic ESP32 input-only GPIOs (cannot drive heater/fan outputs).
  static const inputOnlyEsp32 = {34, 35, 36, 37, 38, 39};

  /// Common strapping pins that should be warned/blocked for outputs.
  static const strappingEsp32 = {0, 2, 12, 15};

  PinValidationResult validate(DeviceProfile profile) {
    final errors = <String>[];
    final map = EspGpioMap.forModel(profile.deviceModel);
    final pins = profile.pinMapping.assignedPins().toList();
    final seen = <int, String>{};

    if (!EspGpioMap.supportedModels.contains(profile.deviceModel)) {
      errors.add(
        'Unsupported model ${profile.deviceModel}. Supported: ${EspGpioMap.supportedModels.join(", ")}',
      );
    }

    for (final e in pins) {
      final pin = e.value;
      final name = e.key;
      if (seen.containsKey(pin)) {
        errors.add('Duplicate GPIO $pin: ${seen[pin]} and $name');
      } else {
        seen[pin] = name;
      }

      if (map.flashReserved.contains(pin) || map.psramReserved.contains(pin)) {
        errors.add('GPIO $pin is reserved (flash/PSRAM) on ${map.model}; cannot assign $name');
      }

      if (map.inputOnly.contains(pin) && _isOutput(name)) {
        errors.add('GPIO $pin is input-only on ${map.model}; cannot assign $name');
      }

      if (map.strapping.contains(pin) && _isOutput(name)) {
        errors.add('GPIO $pin is a strapping pin; unsafe for $name');
      }

      if (_isPwm(name) && !map.pwmCapable.contains(pin)) {
        errors.add('GPIO $pin is not PWM-capable on ${map.model} for $name');
      }
    }

    for (final s in profile.sensors) {
      if (s.sensor == 'none') continue;
      if (s.gpioPin != null) {
        final pin = s.gpioPin!;
        if (map.flashReserved.contains(pin) || map.psramReserved.contains(pin)) {
          errors.add('Sensor ${s.role} GPIO $pin is reserved (flash/PSRAM)');
        }
        if (s.sensor == 'ntc' && !map.adcCapable.contains(pin)) {
          errors.add('Sensor ${s.role} (NTC/analog) needs an ADC-capable GPIO; $pin is not');
        }
        if (seen.containsKey(pin) && seen[pin] != 'sensor:${s.role}') {
          final existing = seen[pin]!;
          if (!existing.startsWith('sensor:')) {
            errors.add('Sensor ${s.role} GPIO $pin conflicts with $existing');
          }
        } else {
          seen[pin] = 'sensor:${s.role}';
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

  bool _isPwm(String name) => name.contains('pwm');
}
