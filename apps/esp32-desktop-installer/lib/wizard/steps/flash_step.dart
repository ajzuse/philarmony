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

  Future<void> _runFlash() async {
    if (!widget.controller.canFlash) {
      setState(() {});
      return;
    }
    setState(() {
      running = true;
      logs.clear();
      net = NetworkVerifyStatus.pending;
    });

    final job = FlashJob(startedAt: DateTime.now().toUtc());
    widget.controller.setFlashJob(job);

    final assets = await FlashAssetResolver.resolveFirmwareSet();
    final firmwarePath = assets['firmware.bin'];
    if (firmwarePath == null) {
      job.state = FlashJobState.failed;
      job.error =
          'firmware.bin not found. Run: make sync-installer-firmware';
      job.finishedAt = DateTime.now().toUtc();
      widget.controller.setFlashJob(job);
      setState(() => running = false);
      return;
    }

    final package = FirmwarePackage(
      version: widget.controller.profile.firmwareVersion,
      appBinaryPath: firmwarePath,
      bootloaderPath: assets['bootloader.bin'],
      partitionTablePath: assets['partitions.bin'],
      checksumSha256: 'synced',
    );
    final flasher = EsptoolFirmwareFlasher();
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
      }
      if (ev.stage == FlashJobState.success) {
        job.finishedAt = DateTime.now().toUtc();
        net = await SoftPostFlashVerifier().tryReach();
        job.networkVerify = net;
        await LastProfileStore().save(widget.controller.profile);
      }
      widget.controller.setFlashJob(job);
      setState(() {});
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
    final job = widget.controller.session.flashJob;
    final port = widget.controller.selectedPortPath ?? '(none)';
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text('Serial port: $port'),
        const SizedBox(height: 8),
        LinearProgressIndicator(value: (job?.progressPct ?? 0) / 100.0),
        Text('State: ${job?.state.name ?? 'idle'}'),
        if (job?.isSuccess == true) ...[
          const Text('Flash Success'),
          if (net == NetworkVerifyStatus.warn ||
              net == NetworkVerifyStatus.skipped)
            Text(
              'Device not reachable on network (optional check). Flash still succeeded.',
              style: TextStyle(color: Theme.of(context).colorScheme.tertiary),
            ),
        ],
        if (job?.isFailed == true) ...[
          Text(job?.error ?? 'Failed',
              style: TextStyle(color: Theme.of(context).colorScheme.error)),
          const Text(
            'Retry re-runs a full erase→write→verify of the current package. '
            'No previous firmware image is restored automatically.',
          ),
          FilledButton(
            onPressed: running ? null : _runFlash,
            child: const Text('Retry'),
          ),
        ],
        const SizedBox(height: 8),
        Expanded(child: FlashLogPanel(lines: logs)),
      ],
    );
  }
}
