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

import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../l10n/app_localizations.dart';
import '../installer_session_controller.dart';

class SensorsStep extends StatefulWidget {
  const SensorsStep({super.key, required this.controller});
  final InstallerSessionController controller;

  @override
  State<SensorsStep> createState() => _SensorsStepState();
}

class _SensorsStepState extends State<SensorsStep> {
  bool advanced = false;
  String advancedTarget = 'temperature';

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final sensors = widget.controller.profile.sensors;
    SensorConfig temp = sensors.firstWhere(
      (s) => s.role == 'temperature',
      orElse: () => SensorConfig(role: 'temperature', sensor: 'none'),
    );
    SensorConfig hum = sensors.firstWhere(
      (s) => s.role == 'humidity',
      orElse: () => SensorConfig(role: 'humidity', sensor: 'none'),
    );

    void save() {
      widget.controller.updateProfile(
        (p) => p
          ..sensors = [
            temp,
            hum,
          ],
      );
    }

    final target = advancedTarget == 'humidity' ? hum : temp;

    return ListView(
      children: [
        Text('${l10n?.tempSensorLabel ?? 'Temperature sensor'} *'),
        Text(l10n?.requiredField ?? 'Required',
            style: Theme.of(context).textTheme.bodySmall),
        DropdownButton<String>(
          value: temp.sensor,
          items: const [
            'none',
            'dht22',
            'ds18b20',
            'bme280',
            'sht3x',
            'ntc',
            'aht20'
          ].map((e) => DropdownMenuItem(value: e, child: Text(e))).toList(),
          onChanged: (v) {
            temp.sensor = v ?? 'none';
            save();
          },
        ),
        TextFormField(
          decoration: InputDecoration(
            labelText: l10n?.tempGpioLabel ?? 'Temp GPIO',
            helperText: temp.sensor == 'none'
                ? (l10n?.optionalField ?? 'Optional')
                : (l10n?.requiredField ?? 'Required'),
          ),
          initialValue: temp.gpioPin?.toString() ?? '',
          onChanged: (v) {
            temp.gpioPin = int.tryParse(v);
            save();
          },
        ),
        const SizedBox(height: 16),
        Text(l10n?.humiditySensorLabel ?? 'Humidity sensor'),
        Text(l10n?.optionalField ?? 'Optional',
            style: Theme.of(context).textTheme.bodySmall),
        DropdownButton<String>(
          value: hum.sensor,
          items: const ['none', 'dht22', 'bme280', 'sht3x', 'aht20']
              .map((e) => DropdownMenuItem(value: e, child: Text(e)))
              .toList(),
          onChanged: (v) {
            hum.sensor = v ?? 'none';
            save();
          },
        ),
        TextFormField(
          decoration: InputDecoration(
            labelText: l10n?.humidityGpioLabel ?? 'Humidity GPIO',
            helperText: l10n?.optionalField ?? 'Optional',
          ),
          initialValue: hum.gpioPin?.toString() ?? '',
          onChanged: (v) {
            hum.gpioPin = int.tryParse(v);
            save();
          },
        ),
        SwitchListTile(
          title: Text(l10n?.advancedDriverTitle ?? 'Advanced: custom JSON driver'),
          subtitle: Text(
            l10n?.advancedDriverSubtitle ??
                'Optional — merges into sensor parameters',
          ),
          value: advanced,
          onChanged: (v) => setState(() => advanced = v),
        ),
        if (advanced) ...[
          DropdownButtonFormField<String>(
            initialValue: advancedTarget,
            decoration: InputDecoration(
              labelText: l10n?.driverTargetLabel ?? 'Apply JSON to',
              helperText: l10n?.optionalField ?? 'Optional',
            ),
            items: const [
              DropdownMenuItem(value: 'temperature', child: Text('Temperature')),
              DropdownMenuItem(value: 'humidity', child: Text('Humidity')),
            ],
            onChanged: (v) => setState(() => advancedTarget = v ?? 'temperature'),
          ),
          TextFormField(
            decoration: InputDecoration(
              labelText: l10n?.driverJsonLabel ?? 'Driver JSON',
              helperText: '{"driver":"…","options":{…}}',
            ),
            maxLines: 4,
            initialValue: const JsonEncoder.withIndent('  ').convert(
              target.parameters.isEmpty ? {'driver': ''} : target.parameters,
            ),
            onChanged: (v) {
              try {
                final map = jsonDecode(v) as Map<String, dynamic>;
                if (advancedTarget == 'humidity') {
                  hum.parameters = map;
                } else {
                  temp.parameters = map;
                }
                save();
              } catch (_) {
                // keep typing until valid JSON
              }
            },
          ),
        ],
        if (widget.controller.session.validationErrors.isNotEmpty)
          Text(
            widget.controller.session.validationErrors.join('\n'),
            style: TextStyle(color: Theme.of(context).colorScheme.error),
          ),
      ],
    );
  }
}
