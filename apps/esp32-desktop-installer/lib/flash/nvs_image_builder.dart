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
import 'dart:io';

import 'package:path/path.dart' as p;
import 'package:philarmony_core/philarmony_core.dart';

/// Builds a firmware-compatible NVS partition image (`filament_dryer` namespace).
class NvsImageBuilder {
  NvsImageBuilder({NvsConfigMapper? mapper, NvsBinaryWriter? writer})
      : _mapper = mapper ?? NvsConfigMapper(),
        _writer = writer ?? NvsBinaryWriter();

  final NvsConfigMapper _mapper;
  final NvsBinaryWriter _writer;

  /// Writes a 20 KiB NVS image for `esptool write_flash 0x9000`.
  Future<String> writeTempConfig(DeviceProfile profile) async {
    final dir = await Directory.systemTemp.createTemp('philarmony_nvs_');
    final file = File(p.join(dir.path, 'nvs.bin'));
    final entries = _mapper.toPreferencesStringEntries(profile);
    final bytes = _writer.build(
      namespace: 'filament_dryer',
      entries: entries,
      sizeBytes: 0x5000,
    );
    await file.writeAsBytes(bytes, flush: true);
    // Sidecar JSON kept for debugging (not flashed).
    await File(p.join(dir.path, 'config.debug.json')).writeAsString(
      const JsonEncoder.withIndent('  ').convert(_mapper.toFlashConfigBlob(profile)),
    );
    return file.path;
  }
}
