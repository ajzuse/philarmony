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

import '../installer_session_controller.dart';

class SensorsStep extends StatefulWidget {
  const SensorsStep({super.key, required this.controller});
  final InstallerSessionController controller;

  @override
  State<SensorsStep> createState() => _SensorsStepState();
}

class _SensorsStepState extends State<SensorsStep> {
  bool advanced = false;

  @override
  Widget build(BuildContext context) {
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

    return ListView(
      children: [
        const Text('Temperature sensor *'),
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
          decoration: const InputDecoration(labelText: 'Temp GPIO'),
          initialValue: temp.gpioPin?.toString() ?? '',
          onChanged: (v) {
            temp.gpioPin = int.tryParse(v);
            save();
          },
        ),
        const SizedBox(height: 16),
        const Text('Humidity sensor'),
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
          decoration: const InputDecoration(labelText: 'Humidity GPIO'),
          initialValue: hum.gpioPin?.toString() ?? '',
          onChanged: (v) {
            hum.gpioPin = int.tryParse(v);
            save();
          },
        ),
        SwitchListTile(
          title: const Text('Advanced: custom JSON driver'),
          subtitle: const Text('Optional — merges into sensor parameters'),
          value: advanced,
          onChanged: (v) => setState(() => advanced = v),
        ),
        if (advanced)
          TextFormField(
            decoration: const InputDecoration(
              labelText: 'Driver JSON',
              helperText: '{"driver":"…","options":{…}}',
            ),
            maxLines: 4,
            initialValue: const JsonEncoder.withIndent('  ')
                .convert(temp.parameters.isEmpty ? {'driver': ''} : temp.parameters),
            onChanged: (v) {
              try {
                final map = jsonDecode(v) as Map<String, dynamic>;
                temp.parameters = map;
                save();
              } catch (_) {
                // keep typing until valid JSON
              }
            },
          ),
      ],
    );
  }
}
