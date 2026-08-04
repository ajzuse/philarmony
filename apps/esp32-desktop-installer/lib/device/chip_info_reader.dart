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

import 'dart:io';

import '../flash/flash_asset_resolver.dart';

class ChipProbeResult {
  const ChipProbeResult({
    required this.chipName,
    this.flashSizeMb,
    this.raw = '',
  });

  final String chipName;
  final int? flashSizeMb;
  final String raw;
}

/// Probes connected ESP via bundled esptool — never dumps NVS (clarify R9).
class ChipInfoReader {
  Future<String?> describePort(String portPath) async {
    final r = await probe(portPath);
    if (r == null) return null;
    final size = r.flashSizeMb != null ? ' · ${r.flashSizeMb}MB' : '';
    return '${r.chipName}$size @$portPath';
  }

  Future<ChipProbeResult?> probe(String portPath, {int baud = 921600}) async {
    if (portPath.isEmpty) return null;
    try {
      final esptool = await FlashAssetResolver.resolveEsptool();
      final proc = await Process.run(esptool, [
        '--port',
        portPath,
        '--baud',
        '$baud',
        'flash_id',
      ]);
      final out = '${proc.stdout}\n${proc.stderr}';
      if (proc.exitCode != 0 && out.trim().isEmpty) return null;
      return _parse(out);
    } catch (_) {
      return null;
    }
  }

  ChipProbeResult _parse(String out) {
    var chip = 'ESP32';
    final upper = out.toUpperCase();
    if (upper.contains('ESP32-S3')) {
      chip = 'ESP32-S3';
    } else if (upper.contains('ESP32-S2')) {
      chip = 'ESP32-S2';
    } else if (upper.contains('ESP32-C3')) {
      chip = 'ESP32-C3';
    } else if (upper.contains('ESP32')) {
      chip = 'ESP32';
    }

    int? mb;
    final sizeMatch =
        RegExp(r'Detected flash size:\s*(\d+)\s*MB', caseSensitive: false)
            .firstMatch(out);
    if (sizeMatch != null) {
      mb = int.tryParse(sizeMatch.group(1)!);
    }
    return ChipProbeResult(chipName: chip, flashSizeMb: mb, raw: out);
  }
}
