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

import 'package:flutter/material.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../l10n/app_localizations.dart';
import '../installer_session_controller.dart';

class PinsStep extends StatelessWidget {
  const PinsStep({super.key, required this.controller});
  final InstallerSessionController controller;

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final pins = controller.profile.pinMapping;
    final model = controller.profile.deviceModel;
    final map = EspGpioMap.forModel(model);
    Widget pinField(String label, int? value, void Function(int?) set,
        {bool required = false}) {
      return TextFormField(
        decoration: InputDecoration(
          labelText: required ? '$label *' : label,
          helperText: required
              ? (l10n?.requiredField ?? 'Required')
              : (l10n?.optionalField ?? 'Optional'),
        ),
        initialValue: value?.toString() ?? '',
        onChanged: (v) {
          set(int.tryParse(v));
          controller.updateProfile((p) => p);
        },
      );
    }

    return ListView(
      children: [
        Text(
          '${l10n?.pinoutTitle ?? 'Pinout'} — $model',
          style: Theme.of(context).textTheme.titleMedium,
        ),
        const SizedBox(height: 8),
        _EspPinoutDiagram(model: model, pins: pins, map: map),
        const SizedBox(height: 16),
        pinField(l10n?.heaterPwmLabel ?? 'Heater PWM', pins.heaterPwm, (v) {
          if (v != null) pins.heaterPwm = v;
        }, required: true),
        pinField(l10n?.exhaustFanPwmLabel ?? 'Exhaust fan PWM', pins.exhaustFanPwm,
            (v) => pins.exhaustFanPwm = v),
        pinField(
            l10n?.exhaustFanDigitalLabel ?? 'Exhaust fan digital',
            pins.exhaustFanDigital,
            (v) => pins.exhaustFanDigital = v),
        pinField('I2C SDA', pins.i2cSda, (v) => pins.i2cSda = v),
        pinField('I2C SCL', pins.i2cScl, (v) => pins.i2cScl = v),
        pinField('SPI MOSI', pins.spiMosi, (v) => pins.spiMosi = v),
        pinField('SPI MISO', pins.spiMiso, (v) => pins.spiMiso = v),
        pinField('SPI SCK', pins.spiSck, (v) => pins.spiSck = v),
        pinField('SPI CS', pins.spiCs, (v) => pins.spiCs = v),
        pinField('SPI DC', pins.spiDc, (v) => pins.spiDc = v),
        pinField('SPI RESET', pins.spiReset, (v) => pins.spiReset = v),
        if (controller.session.validationErrors.isNotEmpty)
          Text(
            controller.session.validationErrors.join('\n'),
            style: TextStyle(color: Theme.of(context).colorScheme.error),
          ),
        const SizedBox(height: 8),
        Text(
          'Strapping: ${map.strapping.join(", ")}. '
          'Flash reserved: ${map.flashReserved.join(", ")}. '
          'PSRAM reserved: ${map.psramReserved.isEmpty ? "—": map.psramReserved.join(", ")}. '
          'Input-only: ${map.inputOnly.isEmpty ? "—": map.inputOnly.join(", ")}.',
          style: Theme.of(context).textTheme.bodySmall,
        ),
      ],
    );
  }
}

class _EspPinoutDiagram extends StatelessWidget {
  const _EspPinoutDiagram({
    required this.model,
    required this.pins,
    required this.map,
  });
  final String model;
  final PinMapping pins;
  final EspGpioMap map;

  @override
  Widget build(BuildContext context) {
    final scheme = Theme.of(context).colorScheme;
    final gpios = map.usableGpios;
    final mid = (gpios.length / 2).ceil();
    final left = gpios.take(mid).toList();
    final right = gpios.skip(mid).toList();
    final assigned = {
      for (final e in pins.assignedPins()) e.value: e.key,
    };

    Widget gpioCell(int gpio) {
      final role = assigned[gpio];
      final isStrapping = map.strapping.contains(gpio);
      final isInputOnly = map.inputOnly.contains(gpio);
      final isFlash = map.flashReserved.contains(gpio);
      final isPsram = map.psramReserved.contains(gpio);
      Color bg = scheme.surfaceContainerHighest;
      if (role != null) bg = scheme.primaryContainer;
      if (isStrapping) bg = scheme.tertiaryContainer;
      if (isInputOnly) bg = scheme.errorContainer.withValues(alpha: 0.5);
      if (isFlash || isPsram) bg = scheme.secondaryContainer;
      return Container(
        width: 72,
        margin: const EdgeInsets.all(2),
        padding: const EdgeInsets.symmetric(vertical: 4, horizontal: 2),
        decoration: BoxDecoration(
          color: bg,
          borderRadius: BorderRadius.circular(4),
          border: Border.all(color: scheme.outlineVariant),
        ),
        child: Column(
          children: [
            Text('G$gpio',
                style:
                    const TextStyle(fontSize: 11, fontWeight: FontWeight.bold)),
            if (role != null)
              Text(role,
                  style: const TextStyle(fontSize: 8),
                  textAlign: TextAlign.center),
            if (isFlash)
              const Text('FLASH', style: TextStyle(fontSize: 7)),
            if (isPsram)
              const Text('PSRAM', style: TextStyle(fontSize: 7)),
          ],
        ),
      );
    }

    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        border: Border.all(color: scheme.outlineVariant),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Column(
        children: [
          Text('$model package outline (GPIO map)',
              style: Theme.of(context).textTheme.labelLarge),
          const SizedBox(height: 8),
          Row(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Expanded(
                  child: Column(children: left.map(gpioCell).toList())),
              Container(
                width: 48,
                height: 200,
                margin: const EdgeInsets.symmetric(horizontal: 8),
                decoration: BoxDecoration(
                  color: scheme.surfaceContainerHigh,
                  borderRadius: BorderRadius.circular(6),
                ),
                alignment: Alignment.center,
                child: RotatedBox(
                  quarterTurns: 3,
                  child: Text(model,
                      style: const TextStyle(fontWeight: FontWeight.bold)),
                ),
              ),
              Expanded(
                  child: Column(children: right.map(gpioCell).toList())),
            ],
          ),
          const SizedBox(height: 8),
          Wrap(
            spacing: 8,
            children: [
              Chip(
                label: const Text('Assigned'),
                visualDensity: VisualDensity.compact,
                backgroundColor: scheme.primaryContainer,
              ),
              Chip(
                label: const Text('Strapping'),
                visualDensity: VisualDensity.compact,
                backgroundColor: scheme.tertiaryContainer,
              ),
              Chip(
                label: const Text('Flash/PSRAM'),
                visualDensity: VisualDensity.compact,
                backgroundColor: scheme.secondaryContainer,
              ),
              Chip(
                label: const Text('Input-only'),
                visualDensity: VisualDensity.compact,
                backgroundColor: scheme.errorContainer.withValues(alpha: 0.5),
              ),
            ],
          ),
        ],
      ),
    );
  }
}
