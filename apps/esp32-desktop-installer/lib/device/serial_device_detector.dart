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
  /// Known USB-UART bridge chips (VID, PID optional).
  static const Map<int, String> _vendorHints = {
    0x10C4: 'CP210x', // Silicon Labs
    0x1A86: 'CH340', // WCH
    0x0403: 'FTDI',
    0x067B: 'Prolific',
  };

  static const Map<(int, int), String> _exactHints = {
    (0x10C4, 0xEA60): 'CP2102/CP210x',
    (0x10C4, 0xEA70): 'CP2105',
    (0x1A86, 0x7523): 'CH340',
    (0x1A86, 0x55D4): 'CH9102',
    (0x0403, 0x6001): 'FT232',
    (0x0403, 0x6015): 'FT231X',
  };

  @override
  Future<List<SerialPortInfo>> listPorts() async {
    try {
      final names = SerialPort.availablePorts;
      return names.map((path) {
        String description = path;
        int? vid;
        int? pid;
        try {
          final port = SerialPort(path);
          final desc = port.description;
          if (desc != null && desc.isNotEmpty) description = desc;
          try {
            vid = port.vendorId;
            pid = port.productId;
          } catch (_) {}
          port.dispose();
        } catch (_) {}
        final hint = usbUartHint(vid: vid, pid: pid);
        if (hint != null) {
          description = '$description ($hint)';
        }
        return SerialPortInfo(
          path: path,
          description: description,
          vid: vid,
          pid: pid,
        );
      }).toList();
    } catch (_) {
      return const <SerialPortInfo>[];
    }
  }

  /// Map VID/PID to a user-facing USB-UART chip hint.
  static String? usbUartHint({int? vid, int? pid}) {
    if (vid == null) return null;
    if (pid != null) {
      final exact = _exactHints[(vid, pid)];
      if (exact != null) return exact;
    }
    return _vendorHints[vid];
  }
}
