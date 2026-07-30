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

import 'package:flutter_libserialport/flutter_libserialport.dart';
import 'package:philarmony_core/philarmony_core.dart';

/// USB-UART enumeration via flutter_libserialport.
class SerialDeviceDetector implements DeviceDetector {
  @override
  Future<List<SerialPortInfo>> listPorts() async {
    try {
      final names = SerialPort.availablePorts;
      return names.map((path) {
        String description = path;
        try {
          final port = SerialPort(path);
          final desc = port.description;
          if (desc != null && desc.isNotEmpty) description = desc;
          port.dispose();
        } catch (_) {}
        return SerialPortInfo(path: path, description: description);
      }).toList();
    } catch (_) {
      return const <SerialPortInfo>[];
    }
  }
}
