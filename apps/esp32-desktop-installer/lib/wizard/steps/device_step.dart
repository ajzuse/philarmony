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

import 'package:file_picker/file_picker.dart';
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
  String? modelWarning;

  static const _baudChoices = [115200, 230400, 460800, 921600];

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
    } else {
      widget.controller.setSelectedPort(null);
    }
  }

  Future<void> _probe(String path) async {
    setState(() {
      probing = true;
      chipLabel = null;
      modelWarning = null;
    });
    final info = await chipReader.probe(
      path,
      baud: widget.controller.session.baudRate,
    );
    if (!mounted) return;
    if (info != null) {
      final supported = EspGpioMap.supportedModels.contains(info.chipName);
      widget.controller.updateProfile((pr) {
        if (supported) {
          pr.deviceModel = info.chipName;
        }
        if (info.flashSizeMb != null &&
            const [4, 8, 16].contains(info.flashSizeMb)) {
          pr.flashSizeMb = info.flashSizeMb!;
        }
        return pr;
      });
      setState(() {
        chipLabel =
            '${info.chipName}${info.flashSizeMb != null ? ' · ${info.flashSizeMb}MB flash' : ''}';
        if (!supported) {
          modelWarning =
              'Unsupported chip ${info.chipName}. Supported variants: '
              '${EspGpioMap.supportedModels.join(", ")}. Select a supported model manually.';
        }
        probing = false;
      });
    } else {
      setState(() {
        chipLabel = 'Could not auto-detect chip (select model manually)';
        probing = false;
      });
    }
  }

  Future<void> _pickPartition() async {
    final result = await FilePicker.platform.pickFiles(
      dialogTitle: 'Select custom partitions.bin',
      type: FileType.any,
    );
    final path = result?.files.single.path;
    if (path != null) {
      widget.controller.setCustomPartitionTablePath(path);
      setState(() {});
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final p = widget.controller.profile;
    final selected = widget.controller.selectedPortPath;
    final baud = widget.controller.session.baudRate;
    final errors = widget.controller.session.validationErrors;
    return ListView(
      children: [
        DropdownButtonFormField<String>(
          key: ValueKey('model-${p.deviceModel}'),
          initialValue: EspGpioMap.supportedModels.contains(p.deviceModel)
              ? p.deviceModel
              : 'ESP32',
          decoration: InputDecoration(
            labelText: '${l10n?.deviceModelLabel ?? 'ESP32 model'} *',
            helperText: l10n?.requiredField ?? 'Required',
          ),
          items: EspGpioMap.supportedModels
              .map((e) => DropdownMenuItem(value: e, child: Text(e)))
              .toList(),
          onChanged: (v) {
            if (v == null) return;
            widget.controller.updateProfile((pr) => pr..deviceModel = v);
            setState(() => modelWarning = null);
          },
        ),
        DropdownButtonFormField<int>(
          key: ValueKey('flash-${p.flashSizeMb}'),
          initialValue: p.flashSizeMb,
          decoration: InputDecoration(
            labelText: '${l10n?.flashSizeLabel ?? 'Flash size (MB)'} *',
            helperText: l10n?.requiredField ?? 'Required',
          ),
          items: const [4, 8, 16]
              .map((e) => DropdownMenuItem(value: e, child: Text('$e')))
              .toList(),
          onChanged: (v) {
            if (v == null) return;
            widget.controller.updateProfile((pr) => pr..flashSizeMb = v);
          },
        ),
        DropdownButtonFormField<int>(
          key: ValueKey('baud-$baud'),
          initialValue: _baudChoices.contains(baud) ? baud : 921600,
          decoration: InputDecoration(
            labelText: '${l10n?.baudLabel ?? 'Baud rate'} *',
            helperText: l10n?.optionalAdvanced ?? 'Default 921600 (configurable)',
          ),
          items: _baudChoices
              .map((e) => DropdownMenuItem(value: e, child: Text('$e')))
              .toList(),
          onChanged: (v) async {
            if (v == null) return;
            widget.controller.setBaudRate(v);
            final port = widget.controller.selectedPortPath;
            if (port != null) await _probe(port);
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
                decoration: InputDecoration(
                  labelText: '${l10n?.usbPortLabel ?? 'USB port'} *',
                  helperText: l10n?.requiredField ?? 'Required',
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
        const SizedBox(height: 8),
        ListTile(
          contentPadding: EdgeInsets.zero,
          title: Text(l10n?.customPartitionLabel ?? 'Custom partition table'),
          subtitle: Text(
            widget.controller.session.customPartitionTablePath ??
                (l10n?.optionalField ?? 'Optional — default bundled partitions.bin'),
          ),
          trailing: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              IconButton(
                tooltip: 'Clear',
                onPressed: () {
                  widget.controller.setCustomPartitionTablePath(null);
                  setState(() {});
                },
                icon: const Icon(Icons.clear),
              ),
              IconButton(
                onPressed: _pickPartition,
                icon: const Icon(Icons.folder_open),
              ),
            ],
          ),
        ),
        if (probing) const LinearProgressIndicator(),
        if (chipLabel != null)
          Padding(
            padding: const EdgeInsets.only(top: 8),
            child: Text(chipLabel!),
          ),
        if (modelWarning != null)
          Padding(
            padding: const EdgeInsets.only(top: 8),
            child: Text(
              modelWarning!,
              style: TextStyle(color: Theme.of(context).colorScheme.error),
            ),
          ),
        if (ports.isEmpty)
          Padding(
            padding: const EdgeInsets.only(top: 12),
            child: Text(l10n?.noPorts ??
                'No USB serial ports found. Install CH340/CP210x/FTDI drivers and reconnect.'),
          ),
        if (errors.isNotEmpty)
          Padding(
            padding: const EdgeInsets.only(top: 12),
            child: Text(
              errors.join('\n'),
              style: TextStyle(color: Theme.of(context).colorScheme.error),
            ),
          ),
      ],
    );
  }
}
