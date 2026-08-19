/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import 'advanced_section.dart';
import 'config_controller.dart';
import 'pinout_board.dart';
import 'wifi_section.dart';

const _sensorTypes = ['sht3x', 'bme280', 'dht22', 'ds18b20', 'ntc'];
const _displayDrivers = ['st7789', 'ssd1306', 'sh1106', 'none'];
const _statusFieldOptions = [
  'status',
  'chamber_temp_c',
  'target_temp_c',
  'humidity_pct',
  'target_humidity_pct',
  'heater_power_pct',
  'elapsed_time_sec',
  'remaining_time_sec',
];

class ConfigPage extends ConsumerWidget {
  const ConfigPage({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final state = ref.watch(configControllerProvider);
    final controller = ref.read(configControllerProvider.notifier);
    final theme = Theme.of(context);

    return Scaffold(
      appBar: AppBar(
        title: const Text('Device configuration'),
        actions: [
          IconButton(
            tooltip: 'Reload from device',
            onPressed: controller.resetDraftFromDevice,
            icon: const Icon(Icons.refresh),
          ),
        ],
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          if (state.saveState == ConfigSaveState.saved)
            const _BannerMessage(
              color: Colors.green,
              message: 'Configuration saved on device.',
            ),
          if (state.saveState == ConfigSaveState.queued)
            const _BannerMessage(
              color: Colors.amber,
              message: 'Configuration queued — will sync when device reconnects.',
            ),
          if (state.saveState == ConfigSaveState.validationFailed)
            _BannerMessage(
              color: theme.colorScheme.errorContainer,
              message: state.validationErrors.join('\n'),
            ),
          if (state.saveState == ConfigSaveState.deviceError && state.deviceError != null)
            _BannerMessage(
              color: theme.colorScheme.errorContainer,
              message: state.deviceError!,
            ),
          PinoutBoard(
            config: state.draft,
            deviceModel: state.deviceModel,
            onHeaterPinChanged: controller.updateHeaterPin,
            onFanPinChanged: controller.updateFanPin,
            onI2cSdaChanged: (pin) {
              final scl = _i2cScl(state.draft) ?? 22;
              controller.updateI2cPins(sda: pin, scl: scl);
            },
            onI2cSclChanged: (pin) {
              final sda = _i2cSda(state.draft) ?? 21;
              controller.updateI2cPins(sda: sda, scl: pin);
            },
          ),
          const SizedBox(height: 12),
          _SensorsSection(
            sensors: state.draft.sensors,
            onTypeChanged: controller.updateSensorType,
            onPinChanged: controller.updateSensorPin,
          ),
          const SizedBox(height: 12),
          _ActuatorsSection(
            actuators: state.draft.actuators,
            onPinChanged: controller.updateActuatorPin,
          ),
          const SizedBox(height: 12),
          _DisplaySection(
            display: state.draft.display,
            onEnabledChanged: controller.updateDisplayEnabled,
            onDriverChanged: controller.updateDisplayDriver,
            onWidthChanged: controller.updateDisplayWidth,
            onHeightChanged: controller.updateDisplayHeight,
            onFieldToggled: controller.updateDisplayField,
            onSpiPinChanged: controller.updateSpiBusPin,
          ),
          const SizedBox(height: 12),
          _SafetyLimitsSection(control: state.draft.control),
          const SizedBox(height: 12),
          const WifiSection(),
          const SizedBox(height: 12),
          const AdvancedSection(),
          const SizedBox(height: 24),
          FilledButton.icon(
            onPressed: state.saveState == ConfigSaveState.saving
                ? null
                : () => controller.save(),
            icon: state.saveState == ConfigSaveState.saving
                ? const SizedBox(
                    width: 18,
                    height: 18,
                    child: CircularProgressIndicator(strokeWidth: 2),
                  )
                : const Icon(Icons.save),
            label: const Text('Save hardware configuration'),
          ),
        ],
      ),
    );
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

class _BannerMessage extends StatelessWidget {
  const _BannerMessage({required this.color, required this.message});

  final Color color;
  final String message;

  @override
  Widget build(BuildContext context) {
    return Container(
      width: double.infinity,
      margin: const EdgeInsets.only(bottom: 12),
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: color,
        borderRadius: BorderRadius.circular(8),
      ),
      child: Text(message),
    );
  }
}

class _SensorsSection extends StatelessWidget {
  const _SensorsSection({
    required this.sensors,
    required this.onTypeChanged,
    required this.onPinChanged,
  });

  final List<Map<String, dynamic>> sensors;
  final void Function(int index, String type) onTypeChanged;
  final void Function(int index, int pin) onPinChanged;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Sensors', style: Theme.of(context).textTheme.titleMedium),
            const SizedBox(height: 8),
            for (var i = 0; i < sensors.length; i++) ...[
              Text(sensors[i]['id'] as String? ?? 'sensor_$i'),
              Row(
                children: [
                  Expanded(
                    child: DropdownButtonFormField<String>(
                      value: _sensorTypes.contains(sensors[i]['type'])
                          ? sensors[i]['type'] as String
                          : _sensorTypes.first,
                      decoration: const InputDecoration(labelText: 'Type'),
                      items: [
                        for (final t in _sensorTypes)
                          DropdownMenuItem(value: t, child: Text(t)),
                      ],
                      onChanged: (v) {
                        if (v != null) onTypeChanged(i, v);
                      },
                    ),
                  ),
                  const SizedBox(width: 12),
                  SizedBox(
                    width: 100,
                    child: TextFormField(
                      initialValue: _sensorPin(sensors[i]).toString(),
                      decoration: const InputDecoration(labelText: 'GPIO'),
                      keyboardType: TextInputType.number,
                      onFieldSubmitted: (v) {
                        final pin = int.tryParse(v);
                        if (pin != null) onPinChanged(i, pin);
                      },
                    ),
                  ),
                ],
              ),
              const Divider(),
            ],
          ],
        ),
      ),
    );
  }

  int _sensorPin(Map<String, dynamic> sensor) {
    final bus = sensor['bus'] as Map<String, dynamic>?;
    if (bus == null) return 0;
    return (bus['pin'] as int?) ?? (bus['sda_pin'] as int?) ?? 0;
  }
}

class _ActuatorsSection extends StatelessWidget {
  const _ActuatorsSection({
    required this.actuators,
    required this.onPinChanged,
  });

  final List<Map<String, dynamic>> actuators;
  final void Function(int index, int pin) onPinChanged;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Pins / actuators', style: Theme.of(context).textTheme.titleMedium),
            const SizedBox(height: 8),
            for (var i = 0; i < actuators.length; i++) ...[
              ListTile(
                contentPadding: EdgeInsets.zero,
                title: Text(actuators[i]['id'] as String? ?? 'actuator_$i'),
                subtitle: Text(
                  '${actuators[i]['type']} · ${actuators[i]['role']}',
                ),
                trailing: SizedBox(
                  width: 88,
                  child: TextFormField(
                    initialValue: _actuatorPin(actuators[i]).toString(),
                    decoration: const InputDecoration(labelText: 'PWM GPIO'),
                    keyboardType: TextInputType.number,
                    onFieldSubmitted: (v) {
                      final pin = int.tryParse(v);
                      if (pin != null) onPinChanged(i, pin);
                    },
                  ),
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }

  int _actuatorPin(Map<String, dynamic> actuator) {
    final pins = actuator['pins'] as Map<String, dynamic>?;
    return (pins?['pwm'] as int?) ?? 0;
  }
}

class _DisplaySection extends StatelessWidget {
  const _DisplaySection({
    required this.display,
    required this.onEnabledChanged,
    required this.onDriverChanged,
    required this.onWidthChanged,
    required this.onHeightChanged,
    required this.onFieldToggled,
    required this.onSpiPinChanged,
  });

  final Map<String, dynamic> display;
  final ValueChanged<bool> onEnabledChanged;
  final ValueChanged<String> onDriverChanged;
  final ValueChanged<int> onWidthChanged;
  final ValueChanged<int> onHeightChanged;
  final void Function(String field, bool enabled) onFieldToggled;
  final void Function(String key, int pin) onSpiPinChanged;

  @override
  Widget build(BuildContext context) {
    final geometry = display['geometry'] as Map<String, dynamic>? ?? {};
    final layout = display['layout'] as Map<String, dynamic>? ?? {};
    final bus = display['bus'] as Map<String, dynamic>? ?? {};
    final enabled = display['enabled'] as bool? ?? false;
    final driver = display['driver'] as String? ?? _displayDrivers.first;
    final selectedFields = (layout['fields'] as List?)
            ?.map((e) => e.toString())
            .toSet() ??
        {};

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Display', style: Theme.of(context).textTheme.titleMedium),
            SwitchListTile(
              contentPadding: EdgeInsets.zero,
              title: const Text('Enabled'),
              value: enabled,
              onChanged: onEnabledChanged,
            ),
            DropdownButtonFormField<String>(
              value: _displayDrivers.contains(driver) ? driver : _displayDrivers.first,
              decoration: const InputDecoration(labelText: 'Driver'),
              items: [
                for (final d in _displayDrivers)
                  DropdownMenuItem(value: d, child: Text(d)),
              ],
              onChanged: (v) {
                if (v != null) onDriverChanged(v);
              },
            ),
            Row(
              children: [
                Expanded(
                  child: TextFormField(
                    initialValue: (geometry['width'] as int? ?? 135).toString(),
                    decoration: const InputDecoration(labelText: 'Width'),
                    keyboardType: TextInputType.number,
                    onFieldSubmitted: (v) {
                      final width = int.tryParse(v);
                      if (width != null) onWidthChanged(width);
                    },
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: TextFormField(
                    initialValue: (geometry['height'] as int? ?? 240).toString(),
                    decoration: const InputDecoration(labelText: 'Height'),
                    keyboardType: TextInputType.number,
                    onFieldSubmitted: (v) {
                      final height = int.tryParse(v);
                      if (height != null) onHeightChanged(height);
                    },
                  ),
                ),
              ],
            ),
            const SizedBox(height: 12),
            Text('Status fields', style: Theme.of(context).textTheme.titleSmall),
            for (final field in _statusFieldOptions)
              CheckboxListTile(
                contentPadding: EdgeInsets.zero,
                title: Text(field),
                value: selectedFields.contains(field),
                onChanged: (v) => onFieldToggled(field, v ?? false),
              ),
            const SizedBox(height: 8),
            Text('SPI bus pins', style: Theme.of(context).textTheme.titleSmall),
            Wrap(
              spacing: 8,
              runSpacing: 8,
              children: [
                for (final entry in const [
                  ('mosi', 'MOSI'),
                  ('sclk', 'SCLK'),
                  ('cs', 'CS'),
                  ('dc', 'DC'),
                  ('rst', 'RST'),
                  ('bl', 'Backlight'),
                ])
                  SizedBox(
                    width: 120,
                    child: TextFormField(
                      initialValue: (bus[entry.$1] as int? ?? 0).toString(),
                      decoration: InputDecoration(labelText: entry.$2),
                      keyboardType: TextInputType.number,
                      onFieldSubmitted: (v) {
                        final pin = int.tryParse(v);
                        if (pin != null) onSpiPinChanged(entry.$1, pin);
                      },
                    ),
                  ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _SafetyLimitsSection extends StatelessWidget {
  const _SafetyLimitsSection({required this.control});

  final Map<String, dynamic> control;

  @override
  Widget build(BuildContext context) {
    final safety = control['safety_limits'] as Map<String, dynamic>? ?? {};
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Safety limits (read-only)', style: Theme.of(context).textTheme.titleMedium),
            const SizedBox(height: 8),
            for (final entry in safety.entries)
              ListTile(
                contentPadding: EdgeInsets.zero,
                title: Text(entry.key),
                trailing: Text('${entry.value}'),
              ),
            if (safety.isEmpty)
              const Text('No safety limits reported by device.'),
          ],
        ),
      ),
    );
  }
}
