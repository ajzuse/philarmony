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

import '../installer_session_controller.dart';

class DisplayStep extends StatelessWidget {
  const DisplayStep({super.key, required this.controller});
  final InstallerSessionController controller;

  static const _presets = <String, (int, int)>{
    '128x64 (OLED)': (128, 64),
    '128x32 (OLED)': (128, 32),
    '240x240 (ST7789)': (240, 240),
    '320x240 (ILI9341)': (320, 240),
  };

  static const _fieldOptions = [
    'chamber_temp_c',
    'target_temp_c',
    'humidity_pct',
    'heater_power_pct',
    'fan_power_pct',
    'status',
    'time',
    'cpu_pct',
    'ram_pct',
  ];

  @override
  Widget build(BuildContext context) {
    final d = controller.profile.display;
    final pins = controller.profile.pinMapping;
    return ListView(
      children: [
        SwitchListTile(
          title: const Text('Enable display'),
          subtitle: const Text('Optional'),
          value: d.enabled,
          onChanged: (v) {
            d.enabled = v;
            controller.updateProfile((p) => p);
          },
        ),
        DropdownButtonFormField<String>(
          initialValue: d.driver,
          decoration: const InputDecoration(labelText: 'Driver'),
          items: const ['none', 'ssd1306', 'sh1106', 'st7789', 'ili9341', 'auto']
              .map((e) => DropdownMenuItem(value: e, child: Text(e)))
              .toList(),
          onChanged: (v) {
            d.driver = v ?? 'none';
            controller.updateProfile((p) => p);
          },
        ),
        DropdownButtonFormField<String>(
          decoration: const InputDecoration(labelText: 'Resolution preset'),
          items: [
            const DropdownMenuItem(value: 'custom', child: Text('Custom')),
            ..._presets.keys.map((k) => DropdownMenuItem(value: k, child: Text(k))),
          ],
          onChanged: (v) {
            if (v == null || v == 'custom') return;
            final preset = _presets[v]!;
            d.width = preset.$1;
            d.height = preset.$2;
            controller.updateProfile((p) => p);
          },
        ),
        TextFormField(
          decoration: const InputDecoration(labelText: 'Width'),
          initialValue: '${d.width}',
          onChanged: (v) {
            d.width = int.tryParse(v) ?? d.width;
            controller.updateProfile((p) => p);
          },
        ),
        TextFormField(
          decoration: const InputDecoration(labelText: 'Height'),
          initialValue: '${d.height}',
          onChanged: (v) {
            d.height = int.tryParse(v) ?? d.height;
            controller.updateProfile((p) => p);
          },
        ),
        const SizedBox(height: 8),
        Text('Status fields', style: Theme.of(context).textTheme.titleSmall),
        Wrap(
          spacing: 8,
          children: _fieldOptions.map((f) {
            final selected = d.fields.contains(f);
            return FilterChip(
              label: Text(f),
              selected: selected,
              onSelected: (on) {
                if (on) {
                  d.fields.add(f);
                } else {
                  d.fields.remove(f);
                }
                controller.updateProfile((p) => p);
              },
            );
          }).toList(),
        ),
        const SizedBox(height: 12),
        Text('Layout preview', style: Theme.of(context).textTheme.titleSmall),
        Container(
          margin: const EdgeInsets.only(top: 8),
          padding: const EdgeInsets.all(12),
          decoration: BoxDecoration(
            border: Border.all(color: Theme.of(context).colorScheme.outline),
            borderRadius: BorderRadius.circular(8),
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text('${d.width}×${d.height} · ${d.driver}'),
              const Divider(),
              ...d.fields.map((f) => Text('• $f')),
              if (d.fields.isEmpty) const Text('(no fields)'),
            ],
          ),
        ),
        if (d.driver == 'st7789' || d.driver == 'ili9341') ...[
          const SizedBox(height: 8),
          TextFormField(
            decoration: const InputDecoration(labelText: 'SPI DC pin'),
            initialValue: pins.spiDc?.toString() ?? '',
            onChanged: (v) {
              pins.spiDc = int.tryParse(v);
              controller.updateProfile((p) => p);
            },
          ),
          TextFormField(
            decoration: const InputDecoration(labelText: 'SPI RESET pin'),
            initialValue: pins.spiReset?.toString() ?? '',
            onChanged: (v) {
              pins.spiReset = int.tryParse(v);
              controller.updateProfile((p) => p);
            },
          ),
        ],
      ],
    );
  }
}
