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

import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:philarmony_core/philarmony_core.dart';

import 'flash_asset_resolver.dart';
import 'nvs_image_builder.dart';

/// Drives bundled esptool via [Process]. Stages match flash-pipeline contract.
class EsptoolFirmwareFlasher implements FirmwareFlasher {
  EsptoolFirmwareFlasher({
    NvsImageBuilder? nvsBuilder,
    this.baudRate = 921600,
  }) : _nvsBuilder = nvsBuilder ?? NvsImageBuilder();

  final NvsImageBuilder _nvsBuilder;
  final int baudRate;

  @override
  Stream<FlashProgressEvent> start({
    required String portPath,
    required DeviceProfile profile,
    required FirmwarePackage package,
  }) {
    final controller = StreamController<FlashProgressEvent>();
    () async {
      try {
        void emit(FlashJobState stage, int pct, [String msg = '']) {
          controller.add(FlashProgressEvent(
            stage: stage,
            progressPct: pct,
            message: msg,
          ));
        }

        emit(FlashJobState.erasing, 5, 'preflight');
        final pinCheck = PinValidator().validate(profile);
        if (!pinCheck.ok) {
          throw StateError(pinCheck.errors.join('; '));
        }
        if (!profile.wifi.hasPassword) {
          throw StateError('WiFi password required');
        }
        if (portPath.trim().isEmpty) {
          throw StateError('Serial port required');
        }

        final appPath = package.appBinaryPath;
        final bootPath = package.bootloaderPath;
        final partPath = package.partitionTablePath;
        for (final entry in {
          'firmware.bin': appPath,
          'bootloader.bin': bootPath,
          'partitions.bin': partPath,
        }.entries) {
          final path = entry.value;
          if (path == null || !await File(path).exists()) {
            throw StateError(
              '${entry.key} missing — run: make sync-installer-firmware',
            );
          }
        }

        final esptool = await FlashAssetResolver.resolveEsptool();
        final nvsPath = await _nvsBuilder.writeTempConfig(profile);
        final portArgs = <String>[
          '--port',
          portPath,
          '--baud',
          '$baudRate',
        ];

        emit(FlashJobState.erasing, 15, 'erase_flash');
        await _run(esptool, [...portArgs, 'erase_flash'], controller);

        emit(FlashJobState.writingApp, 30, 'write_flash bootloader@0x1000');
        await _run(esptool, [
          ...portArgs,
          'write_flash',
          '--verify',
          '0x1000',
          bootPath!,
          '0x8000',
          partPath!,
          '0x10000',
          appPath,
        ], controller);

        emit(FlashJobState.writingNvs, 70, 'write_flash nvs @ 0x9000');
        await _run(esptool, [
          ...portArgs,
          'write_flash',
          '--verify',
          '0x9000',
          nvsPath,
        ], controller);

        emit(FlashJobState.verifying, 90, 'verify complete');
        emit(FlashJobState.resetting, 95, 'reset');
        await _run(esptool, [...portArgs, 'run'], controller, allowFail: true);

        emit(FlashJobState.success, 100, 'Flash Success');
        await controller.close();
      } catch (e, st) {
        controller.add(FlashProgressEvent(
          stage: FlashJobState.failed,
          progressPct: 0,
          message: '$e\n$st',
        ));
        await controller.close();
      }
    }();
    return controller.stream;
  }

  Future<void> _run(
    String exe,
    List<String> args,
    StreamController<FlashProgressEvent> log, {
    bool allowFail = false,
  }) async {
    try {
      final proc = await Process.start(exe, args);
      proc.stdout.transform(utf8.decoder).listen((s) {
        final t = s.trim();
        if (t.isEmpty) return;
        log.add(FlashProgressEvent(
          stage: FlashJobState.writingApp,
          progressPct: 50,
          message: t,
        ));
      });
      proc.stderr.transform(utf8.decoder).listen((s) {
        final t = s.trim();
        if (t.isEmpty) return;
        log.add(FlashProgressEvent(
          stage: FlashJobState.writingApp,
          progressPct: 50,
          message: t,
        ));
      });
      final code = await proc.exitCode;
      if (code != 0 && !allowFail) {
        throw ProcessException(exe, args, 'esptool exit $code', code);
      }
    } on ProcessException {
      if (!allowFail) rethrow;
    }
  }
}

/// Soft WS/HTTP check — never fails Flash Success (R8).
class SoftPostFlashVerifier implements PostFlashVerifier {
  @override
  Future<NetworkVerifyStatus> tryReach({
    String? host,
    Duration timeout = const Duration(seconds: 5),
  }) async {
    host = (host == null || host.isEmpty) ? '192.168.4.1' : host;
    try {
      final client = HttpClient();
      client.connectionTimeout = timeout;
      final req = await client
          .getUrl(Uri.parse('http://$host/api/info'))
          .timeout(timeout);
      final res = await req.close().timeout(timeout);
      client.close(force: true);
      if (res.statusCode >= 200 && res.statusCode < 500) {
        return NetworkVerifyStatus.ok;
      }
      return NetworkVerifyStatus.warn;
    } catch (_) {
      return NetworkVerifyStatus.warn;
    }
  }
}
