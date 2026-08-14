/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:philarmony_core/philarmony_core.dart';

/// Visual ESP32 DevKit pin grid with dropdown GPIO assignment (FR-004).
class PinoutBoard extends StatelessWidget {
  const PinoutBoard({
    super.key,
    required this.config,
    required this.deviceModel,
    required this.onHeaterPinChanged,
    required this.onFanPinChanged,
    required this.onI2cSdaChanged,
    required this.onI2cSclChanged,
  });

  final HardwareConfig config;
  final String deviceModel;
  final ValueChanged<int> onHeaterPinChanged;
  final ValueChanged<int> onFanPinChanged;
  final ValueChanged<int> onI2cSdaChanged;
  final ValueChanged<int> onI2cSclChanged;

  static const _devkitPins = <int>[
    36, 39, 34, 35, 32, 33, 25, 26, 27, 14, 12, 13, 9, 10,
    23, 22, 1, 3, 21, 19, 18, 5, 17, 16, 4, 0, 2, 15, 8, 7,
  ];

  @override
  Widget build(BuildContext context) {
    final gpioMap = EspGpioMap.forModel(deviceModel);
    final usable = gpioMap.usableGpios.toSet();
    final pins = _devkitPins.where(usable.contains).toList();

    final heaterPin = _heaterPin(config);
    final fanPin = _fanPin(config);
    final sdaPin = _i2cSda(config);
    final sclPin = _i2cScl(config);

    final assignments = <int, String>{};
    if (heaterPin != null) assignments[heaterPin] = 'Heater';
    if (fanPin != null) assignments[fanPin] = 'Fan';
    if (sdaPin != null) assignments[sdaPin] = 'I2C SDA';
    if (sclPin != null) assignments[sclPin] = 'I2C SCL';

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('ESP32 pinout', style: Theme.of(context).textTheme.titleMedium),
            const SizedBox(height: 4),
            Text(
              'Assign heater, fan, and I2C pins. Validated before save.',
              style: Theme.of(context).textTheme.bodySmall,
            ),
            const SizedBox(height: 12),
            Wrap(
              spacing: 8,
              runSpacing: 8,
              children: [
                for (final pin in pins)
                  _PinChip(
                    pin: pin,
                    label: assignments[pin],
                    reserved: gpioMap.reserved.contains(pin),
                    inputOnly: gpioMap.inputOnly.contains(pin),
                  ),
              ],
            ),
            const Divider(height: 24),
            _PinDropdown(
              label: 'Heater PWM',
              value: heaterPin,
              pins: pins,
              onChanged: onHeaterPinChanged,
            ),
            _PinDropdown(
              label: 'Fan PWM',
              value: fanPin,
              pins: pins,
              onChanged: onFanPinChanged,
            ),
            _PinDropdown(
              label: 'I2C SDA',
              value: sdaPin,
              pins: pins,
              onChanged: onI2cSdaChanged,
            ),
            _PinDropdown(
              label: 'I2C SCL',
              value: sclPin,
              pins: pins,
              onChanged: onI2cSclChanged,
            ),
          ],
        ),
      ),
    );
  }

  static int? _heaterPin(HardwareConfig config) {
    for (final actuator in config.actuators) {
      if (actuator['role'] == 'heater') {
        final pins = actuator['pins'] as Map<String, dynamic>?;
        return (pins?['pwm'] as int?);
      }
    }
    return null;
  }

  static int? _fanPin(HardwareConfig config) {
    for (final actuator in config.actuators) {
      if (actuator['role'] == 'fan') {
        final pins = actuator['pins'] as Map<String, dynamic>?;
        return (pins?['pwm'] as int?);
      }
    }
    return null;
  }

  static int? _i2cSda(HardwareConfig config) {
    for (final sensor in config.sensors) {
      final bus = sensor['bus'] as Map<String, dynamic>?;
      if (bus?['type'] == 'i2c') {
        return (bus?['sda_pin'] as int?) ?? (bus?['pin'] as int?);
      }
    }
    return null;
  }

  static int? _i2cScl(HardwareConfig config) {
    for (final sensor in config.sensors) {
      final bus = sensor['bus'] as Map<String, dynamic>?;
      if (bus?['type'] == 'i2c') {
        return bus?['scl_pin'] as int?;
      }
    }
    return null;
  }
}

class _PinChip extends StatelessWidget {
  const _PinChip({
    required this.pin,
    required this.label,
    required this.reserved,
    required this.inputOnly,
  });

  final int pin;
  final String? label;
  final bool reserved;
  final bool inputOnly;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    Color? fill;
    if (label != null) {
      fill = theme.colorScheme.primaryContainer;
    } else if (reserved) {
      fill = theme.colorScheme.surfaceContainerHighest;
    } else if (inputOnly) {
      fill = theme.colorScheme.tertiaryContainer;
    }

    return Chip(
      label: Text(
        label != null ? 'GPIO $pin\n$label' : 'GPIO $pin',
        textAlign: TextAlign.center,
        style: theme.textTheme.labelSmall,
      ),
      backgroundColor: fill,
      side: BorderSide(
        color: label != null
            ? theme.colorScheme.primary
            : theme.colorScheme.outlineVariant,
      ),
      visualDensity: VisualDensity.compact,
    );
  }
}

class _PinDropdown extends StatelessWidget {
  const _PinDropdown({
    required this.label,
    required this.value,
    required this.pins,
    required this.onChanged,
  });

  final String label;
  final int? value;
  final List<int> pins;
  final ValueChanged<int> onChanged;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: DropdownButtonFormField<int>(
        value: value != null && pins.contains(value) ? value : null,
        decoration: InputDecoration(labelText: label),
        items: [
          for (final pin in pins)
            DropdownMenuItem(value: pin, child: Text('GPIO $pin')),
        ],
        onChanged: (v) {
          if (v != null) onChanged(v);
        },
      ),
    );
  }
}
