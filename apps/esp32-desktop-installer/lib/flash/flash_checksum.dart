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

import 'package:crypto/crypto.dart';

/// Preflight checksum helpers for flash-pipeline contract.
class FlashChecksum {
  /// SHA-256 hex of [path]. Throws if missing/empty.
  static Future<String> sha256File(String path) async {
    final f = File(path);
    if (!await f.exists()) {
      throw StateError('Missing artifact: $path');
    }
    final len = await f.length();
    if (len <= 0) {
      throw StateError('Empty artifact: $path');
    }
    final digest = await sha256.bind(f.openRead()).first;
    return digest.toString();
  }

  /// If `<path>.sha256` exists, require matching digest; otherwise accept computed.
  static Future<String> verifyOrRecord(String path) async {
    final computed = await sha256File(path);
    final sidecar = File('$path.sha256');
    if (await sidecar.exists()) {
      final expected = (await sidecar.readAsString())
          .trim()
          .split(RegExp(r'\s+'))
          .first
          .toLowerCase();
      if (expected.isNotEmpty && expected != computed.toLowerCase()) {
        throw StateError(
          'Checksum mismatch for $path\nexpected=$expected\nactual=$computed',
        );
      }
    }
    return computed;
  }

  static Future<String> combinedPackageChecksum({
    required String firmware,
    required String bootloader,
    required String partitions,
    required String esptool,
  }) async {
    final parts = <String>[
      await verifyOrRecord(firmware),
      await verifyOrRecord(bootloader),
      await verifyOrRecord(partitions),
      await verifyOrRecord(esptool),
    ];
    return sha256.convert(utf8.encode(parts.join(':'))).toString();
  }
}
