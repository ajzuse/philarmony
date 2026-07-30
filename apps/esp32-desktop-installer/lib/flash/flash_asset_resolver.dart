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

import 'package:flutter/services.dart';
import 'package:path/path.dart' as p;
import 'package:path_provider/path_provider.dart';

/// Resolves bundled firmware / esptool paths for Process-based flash.
class FlashAssetResolver {
  static const _firmwareNames = [
    'firmware.bin',
    'bootloader.bin',
    'partitions.bin',
  ];

  /// Copies asset bytes to a support file so esptool can open a real path.
  static Future<String> materializeAsset(
    String assetPath, {
    String? fileName,
  }) async {
    final data = await rootBundle.load(assetPath);
    final bytes = data.buffer.asUint8List();
    final dir = await getApplicationSupportDirectory();
    final outDir = Directory(p.join(dir.path, 'flash_assets'));
    await outDir.create(recursive: true);
    final name = fileName ?? p.basename(assetPath);
    final file = File(p.join(outDir.path, name));
    await file.writeAsBytes(bytes, flush: true);
    return file.path;
  }

  static Future<String?> _resolveNamedBin(String name) async {
    final cwdCandidates = [
      p.join('assets', 'firmware', name),
      p.join(
        'apps',
        'esp32-desktop-installer',
        'assets',
        'firmware',
        name,
      ),
    ];
    for (final c in cwdCandidates) {
      final f = File(c);
      if (await f.exists() && await f.length() > 0) return f.absolute.path;
    }
    try {
      return await materializeAsset('assets/firmware/$name', fileName: name);
    } catch (_) {
      return null;
    }
  }

  static Future<String?> resolveFirmwareBin() => _resolveNamedBin('firmware.bin');

  static Future<String?> resolveBootloaderBin() =>
      _resolveNamedBin('bootloader.bin');

  static Future<String?> resolvePartitionsBin() =>
      _resolveNamedBin('partitions.bin');

  static Future<Map<String, String?>> resolveFirmwareSet() async {
    final out = <String, String?>{};
    for (final n in _firmwareNames) {
      out[n] = await _resolveNamedBin(n);
    }
    return out;
  }

  static Future<String> resolveEsptool() async {
    final os = Platform.isWindows
        ? 'windows'
        : Platform.isMacOS
            ? 'macos'
            : 'linux';
    final exeName = Platform.isWindows ? 'esptool.exe' : 'esptool';
    final assetRel = 'assets/tools/$os/$exeName';

    final cwd = File(assetRel);
    if (await cwd.exists()) {
      await _ensureExecutable(cwd);
      return cwd.absolute.path;
    }

    final exeDir = File(Platform.resolvedExecutable).parent.path;
    final beside = File(p.join(exeDir, 'data', 'flutter_assets', assetRel));
    if (await beside.exists()) {
      await _ensureExecutable(beside);
      return beside.path;
    }

    try {
      final materialized = await materializeAsset(assetRel, fileName: exeName);
      await _ensureExecutable(File(materialized));
      return materialized;
    } catch (_) {
      return Platform.isWindows ? 'esptool.exe' : 'esptool';
    }
  }

  static Future<void> _ensureExecutable(File f) async {
    if (Platform.isWindows) return;
    try {
      await Process.run('chmod', ['+x', f.path]);
    } catch (_) {}
  }
}
