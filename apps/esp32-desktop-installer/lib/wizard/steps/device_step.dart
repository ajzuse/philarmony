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

import '../installer_session_controller.dart';
import '../../device/chip_info_reader.dart';
import '../../device/serial_device_detector.dart';
import '../../l10n/app_localizations.dart';

class DeviceStep extends StatefulWidget {
  const DeviceStep({super.key, required this.controller});
  final InstallerSessionController controller;

  @override
  State<DeviceStep> createState() => _DeviceStepState();
}

class _DeviceStepState extends State<DeviceStep> {
  List<SerialPortInfo> ports = [];
  final detector = SerialDeviceDetector();
  final chipReader = ChipInfoReader();
  String? chipLabel;
  bool probing = false;

  @override
  void initState() {
    super.initState();
    _refresh();
  }

  Future<void> _refresh() async {
    final list = await detector.listPorts();
    setState(() => ports = list);
    final current = widget.controller.selectedPortPath;
    if (current != null && list.any((e) => e.path == current)) {
      await _probe(current);
      return;
    }
    if (list.isNotEmpty) {
      widget.controller.setSelectedPort(list.first.path);
      await _probe(list.first.path);
    }
  }

  Future<void> _probe(String path) async {
    setState(() {
      probing = true;
      chipLabel = null;
    });
    final info = await chipReader.probe(path);
    if (!mounted) return;
    if (info != null) {
      widget.controller.updateProfile((pr) {
        pr.deviceModel = info.chipName;
        if (info.flashSizeMb != null &&
            const [4, 8, 16].contains(info.flashSizeMb)) {
          pr.flashSizeMb = info.flashSizeMb!;
        }
        return pr;
      });
      setState(() {
        chipLabel =
            '${info.chipName}${info.flashSizeMb != null ? ' · ${info.flashSizeMb}MB flash' : ''}';
        probing = false;
      });
    } else {
      setState(() {
        chipLabel = 'Could not auto-detect chip (select model manually)';
        probing = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final p = widget.controller.profile;
    final selected = widget.controller.selectedPortPath;
    return ListView(
      children: [
        DropdownButtonFormField<String>(
          key: ValueKey('model-${p.deviceModel}'),
          initialValue: p.deviceModel,
          decoration: const InputDecoration(
            labelText: 'ESP32 model *',
            helperText: 'Required',
          ),
          items: const ['ESP32', 'ESP32-S2', 'ESP32-S3', 'ESP32-C3']
              .map((e) => DropdownMenuItem(value: e, child: Text(e)))
              .toList(),
          onChanged: (v) {
            if (v == null) return;
            widget.controller.updateProfile((pr) => pr..deviceModel = v);
          },
        ),
        DropdownButtonFormField<int>(
          key: ValueKey('flash-${p.flashSizeMb}'),
          initialValue: p.flashSizeMb,
          decoration: const InputDecoration(
            labelText: 'Flash size (MB) *',
            helperText: 'Required',
          ),
          items: const [4, 8, 16]
              .map((e) => DropdownMenuItem(value: e, child: Text('$e')))
              .toList(),
          onChanged: (v) {
            if (v == null) return;
            widget.controller.updateProfile((pr) => pr..flashSizeMb = v);
          },
        ),
        const SizedBox(height: 12),
        Row(
          children: [
            Expanded(
              child: DropdownButtonFormField<String>(
                key: ValueKey('port-$selected-${ports.length}'),
                initialValue: selected != null &&
                        ports.any((e) => e.path == selected)
                    ? selected
                    : null,
                decoration: const InputDecoration(
                  labelText: 'USB port *',
                  helperText: 'Required',
                ),
                items: ports
                    .map((e) => DropdownMenuItem(
                          value: e.path,
                          child: Text(e.path),
                        ))
                    .toList(),
                onChanged: (v) async {
                  widget.controller.setSelectedPort(v);
                  if (v != null) await _probe(v);
                },
              ),
            ),
            IconButton(onPressed: _refresh, icon: const Icon(Icons.refresh)),
          ],
        ),
        if (probing) const LinearProgressIndicator(),
        if (chipLabel != null)
          Padding(
            padding: const EdgeInsets.only(top: 8),
            child: Text(chipLabel!),
          ),
        if (ports.isEmpty)
          Padding(
            padding: const EdgeInsets.only(top: 12),
            child: Text(l10n?.noPorts ??
                'No USB serial ports found. Install CH340/CP210x/FTDI drivers and reconnect.'),
          ),
      ],
    );
  }
}
