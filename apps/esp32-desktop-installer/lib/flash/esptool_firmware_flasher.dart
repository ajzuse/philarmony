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
import 'package:web_socket_channel/web_socket_channel.dart';

import 'flash_asset_resolver.dart';
import 'flash_checksum.dart';
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

        emit(FlashJobState.preflight, 2, 'preflight');
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
        final packageHash = await FlashChecksum.combinedPackageChecksum(
          firmware: appPath,
          bootloader: bootPath!,
          partitions: partPath!,
          esptool: esptool,
        );
        if (package.checksumSha256.isNotEmpty &&
            package.checksumSha256 != 'synced' &&
            package.checksumSha256 != packageHash) {
          throw StateError(
            'Firmware package checksum mismatch '
            '(expected ${package.checksumSha256}, got $packageHash)',
          );
        }
        emit(FlashJobState.preflight, 8, 'checksum ok $packageHash');

        final nvsPath = await _nvsBuilder.writeTempConfig(profile);
        final portArgs = <String>[
          '--port',
          portPath,
          '--baud',
          '$baudRate',
        ];

        emit(FlashJobState.erasing, 12, 'erase_flash');
        await _run(
          esptool,
          [...portArgs, 'erase_flash'],
          controller,
          stage: FlashJobState.erasing,
          progressBase: 12,
          progressSpan: 15,
        );

        emit(FlashJobState.writingApp, 30, 'write_flash bootloader@0x1000');
        await _run(
          esptool,
          [
            ...portArgs,
            'write_flash',
            '--verify',
            '0x1000',
            bootPath,
            '0x8000',
            partPath,
            '0x10000',
            appPath,
          ],
          controller,
          stage: FlashJobState.writingApp,
          progressBase: 30,
          progressSpan: 35,
        );

        emit(FlashJobState.writingNvs, 70, 'write_flash nvs @ 0x9000');
        await _run(
          esptool,
          [
            ...portArgs,
            'write_flash',
            '--verify',
            '0x9000',
            nvsPath,
          ],
          controller,
          stage: FlashJobState.writingNvs,
          progressBase: 70,
          progressSpan: 15,
        );

        emit(FlashJobState.verifying, 90, 'verify complete');
        emit(FlashJobState.resetting, 95, 'reset');
        await _run(
          esptool,
          [...portArgs, 'run'],
          controller,
          stage: FlashJobState.resetting,
          progressBase: 95,
          progressSpan: 4,
          allowFail: true,
        );

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

  static final _pctRe = RegExp(r'(\d+(?:\.\d+)?)\s*%');

  Future<void> _run(
    String exe,
    List<String> args,
    StreamController<FlashProgressEvent> log, {
    required FlashJobState stage,
    required int progressBase,
    required int progressSpan,
    bool allowFail = false,
  }) async {
    try {
      final proc = await Process.start(exe, args);
      void onChunk(String s) {
        final t = s.trim();
        if (t.isEmpty) return;
        var pct = progressBase;
        final m = _pctRe.firstMatch(t);
        if (m != null) {
          final parsed = double.tryParse(m.group(1)!);
          if (parsed != null) {
            pct = progressBase +
                ((parsed.clamp(0, 100) / 100.0) * progressSpan).round();
          }
        }
        log.add(FlashProgressEvent(
          stage: stage,
          progressPct: pct.clamp(0, 99),
          message: t,
        ));
      }

      proc.stdout.transform(utf8.decoder).listen(onChunk);
      proc.stderr.transform(utf8.decoder).listen(onChunk);
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
    final hosts = <String>[
      if (host != null && host.isNotEmpty) host,
      'philarmony',
      '192.168.4.1',
    ];
    // Deduplicate preserving order
    final seen = <String>{};
    final ordered = <String>[];
    for (final h in hosts) {
      if (seen.add(h)) ordered.add(h);
    }

    var anyAttempt = false;
    for (final h in ordered) {
      anyAttempt = true;
      if (await _tryWebSocket(h, timeout)) {
        return NetworkVerifyStatus.ok;
      }
      if (await _tryHttp(h, timeout)) {
        return NetworkVerifyStatus.ok;
      }
    }
    return anyAttempt ? NetworkVerifyStatus.warn : NetworkVerifyStatus.skipped;
  }

  Future<bool> _tryWebSocket(String h, Duration timeout) async {
    try {
      final uri = Uri.parse('ws://$h/ws');
      final channel = WebSocketChannel.connect(uri);
      await channel.ready.timeout(timeout);
      await channel.sink.close();
      return true;
    } catch (_) {
      return false;
    }
  }

  Future<bool> _tryHttp(String h, Duration timeout) async {
    try {
      final client = HttpClient();
      client.connectionTimeout = timeout;
      final req =
          await client.getUrl(Uri.parse('http://$h/api/info')).timeout(timeout);
      final res = await req.close().timeout(timeout);
      client.close(force: true);
      return res.statusCode >= 200 && res.statusCode < 500;
    } catch (_) {
      return false;
    }
  }

  /// Prefer user static IP, then optional explicit host, then hotspot defaults.
  static String? resolveHost(DeviceProfile profile, {String? explicit}) {
    final staticIp = profile.wifi.staticIp?['ip']?.trim();
    if (staticIp != null && staticIp.isNotEmpty) return staticIp;
    if (explicit != null && explicit.trim().isNotEmpty) return explicit.trim();
    return null; // tryReach will fall through to philarmony / 192.168.4.1
  }
}
