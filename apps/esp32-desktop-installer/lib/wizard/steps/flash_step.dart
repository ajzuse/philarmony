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

import '../../flash/esptool_firmware_flasher.dart';
import '../../flash/flash_asset_resolver.dart';
import '../../flash/flash_checksum.dart';
import '../../flash/flash_error_hints.dart';
import '../../l10n/app_localizations.dart';
import '../../persistence/last_profile_store.dart';
import '../installer_session_controller.dart';
import '../widgets/flash_log_panel.dart';

class FlashStep extends StatefulWidget {
  const FlashStep({super.key, required this.controller});
  final InstallerSessionController controller;

  @override
  State<FlashStep> createState() => _FlashStepState();
}

class _FlashStepState extends State<FlashStep> {
  final logs = <String>[];
  NetworkVerifyStatus net = NetworkVerifyStatus.pending;
  bool running = false;
  String? hint;
  bool partialStateWarn = false;

  Future<void> _runFlash() async {
    if (!widget.controller.canFlash) {
      setState(() {});
      return;
    }
    setState(() {
      running = true;
      logs.clear();
      net = NetworkVerifyStatus.pending;
      hint = null;
      partialStateWarn = false;
    });

    final job = FlashJob(startedAt: DateTime.now().toUtc());
    widget.controller.setFlashJob(job);

    try {
      final assets = await FlashAssetResolver.resolveFirmwareSet();
      final firmwarePath = assets['firmware.bin'];
      if (firmwarePath == null) {
        throw StateError(
          'firmware.bin missing — run: make sync-installer-firmware',
        );
      }
      final boot = assets['bootloader.bin'];
      final part = widget.controller.session.customPartitionTablePath ??
          assets['partitions.bin'];
      if (boot == null || part == null) {
        throw StateError(
          'bootloader.bin/partitions.bin missing — run: make sync-installer-firmware',
        );
      }
      final esptool = await FlashAssetResolver.resolveEsptool();
      final checksum = await FlashChecksum.combinedPackageChecksum(
        firmware: firmwarePath,
        bootloader: boot,
        partitions: part,
        esptool: esptool,
      );

      final package = FirmwarePackage(
        version: widget.controller.profile.firmwareVersion,
        appBinaryPath: firmwarePath,
        bootloaderPath: boot,
        partitionTablePath: part,
        checksumSha256: checksum,
      );
      final flasher = EsptoolFirmwareFlasher(
        baudRate: widget.controller.session.baudRate,
      );
      final port = widget.controller.selectedPortPath!;
      await for (final ev in flasher.start(
        portPath: port,
        profile: widget.controller.profile,
        package: package,
      )) {
        logs.add('[${ev.stage.name}] ${ev.message}');
        job.state = ev.stage;
        job.progressPct = ev.progressPct;
        job.logLines = List.of(logs);
        if (ev.stage == FlashJobState.failed) {
          job.error = ev.message;
          job.finishedAt = DateTime.now().toUtc();
          hint = FlashErrorHints.suggest(ev.message);
          partialStateWarn = FlashErrorHints.looksLikeUsbUnplug(ev.message);
        }
        if (ev.stage == FlashJobState.success) {
          job.finishedAt = DateTime.now().toUtc();
          final host = SoftPostFlashVerifier.resolveHost(
            widget.controller.profile,
          );
          net = await SoftPostFlashVerifier().tryReach(host: host);
          job.networkVerify = net;
          await LastProfileStore().save(widget.controller.profile);
        }
        widget.controller.setFlashJob(job);
        setState(() {});
      }
    } catch (e) {
      job.state = FlashJobState.failed;
      job.error = '$e';
      job.finishedAt = DateTime.now().toUtc();
      hint = FlashErrorHints.suggest('$e');
      partialStateWarn = FlashErrorHints.looksLikeUsbUnplug('$e');
      logs.add('[failed] $e');
      job.logLines = List.of(logs);
      widget.controller.setFlashJob(job);
    }
    setState(() => running = false);
  }

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) => _runFlash());
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final job = widget.controller.session.flashJob;
    final port = widget.controller.selectedPortPath ?? '(none)';
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text('${l10n?.serialPortLabel ?? 'Serial port'}: $port'),
        Text(
          '${l10n?.baudLabel ?? 'Baud'}: ${widget.controller.session.baudRate}',
        ),
        if (widget.controller.session.customPartitionTablePath != null)
          Text(
            '${l10n?.customPartitionLabel ?? 'Custom partition table'}: '
            '${widget.controller.session.customPartitionTablePath}',
          ),
        const SizedBox(height: 8),
        LinearProgressIndicator(value: (job?.progressPct ?? 0) / 100.0),
        Text('State: ${job?.state.name ?? 'idle'}'),
        if (job?.isSuccess == true) ...[
          Text(l10n?.flashSuccess ?? 'Flash Success'),
          if (net == NetworkVerifyStatus.warn ||
              net == NetworkVerifyStatus.skipped)
            Text(
              l10n?.networkWarn ??
                  'Device not reachable on network (optional check). Flash still succeeded.',
              style: TextStyle(color: Theme.of(context).colorScheme.tertiary),
            ),
        ],
        if (job?.isFailed == true) ...[
          Text(job?.error ?? 'Failed',
              style: TextStyle(color: Theme.of(context).colorScheme.error)),
          if (partialStateWarn)
            Text(
              l10n?.partialFlashWarn ??
                  'USB may have disconnected mid-flash. The device may be in a '
                      'partial state — reconnect and Retry a full erase→write→verify.',
              style: TextStyle(color: Theme.of(context).colorScheme.error),
            ),
          if (hint != null) Text(hint!),
          Text(
            l10n?.retryHelp ??
                'Retry re-runs a full erase→write→verify of the current package. '
                    'No previous firmware image is restored automatically.',
          ),
          FilledButton(
            onPressed: running ? null : _runFlash,
            child: Text(l10n?.retry ?? 'Retry'),
          ),
        ],
        const SizedBox(height: 8),
        Expanded(child: FlashLogPanel(lines: logs)),
      ],
    );
  }
}
