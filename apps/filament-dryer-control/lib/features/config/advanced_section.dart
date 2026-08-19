/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../device/session_providers.dart';
import '../../l10n/app_localizations.dart';
import 'config_controller.dart';

class AdvancedSection extends ConsumerWidget {
  const AdvancedSection({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final config = ref.watch(configControllerProvider).draft;
    final status = ref.watch(deviceSessionProvider).status;
    final device = ref.watch(deviceSessionProvider).activeDevice;
    final theme = Theme.of(context);

    final safety = config.control['safety_limits'] as Map<String, dynamic>? ?? {};
    final hardLimit = safety['hard_temp_limit_c'];

    return Semantics(
      label: l10n.advancedSettings,
      container: true,
      child: Card(
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(l10n.advancedSettings, style: theme.textTheme.titleMedium),
              const SizedBox(height: 8),
              _ReadOnlyRow(label: l10n.advancedDeviceName, value: device?.nickname ?? '—'),
              _ReadOnlyRow(label: l10n.manualHost, value: device?.host ?? '—'),
              _ReadOnlyRow(label: l10n.advancedFirmwareVersion, value: l10n.notReportedMvp),
              _ReadOnlyRow(label: l10n.advancedNtpTimezone, value: l10n.notReportedMvp),
              _ReadOnlyRow(
                label: l10n.advancedHardTempLimit,
                value: hardLimit?.toString() ?? '—',
              ),
              _ReadOnlyRow(
                label: l10n.advancedUptimeSec,
                value: status?.uptimeSec.toString() ?? '—',
              ),
              _ReadOnlyRow(
                label: l10n.advancedCpuUsage,
                value: status?.cpuUsagePct?.toStringAsFixed(1) ?? '—',
              ),
              _ReadOnlyRow(
                label: l10n.advancedFreeMemory,
                value: status?.memoryFreeBytes?.toString() ?? '—',
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class _ReadOnlyRow extends StatelessWidget {
  const _ReadOnlyRow({required this.label, required this.value});

  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    return ListTile(
      contentPadding: EdgeInsets.zero,
      title: Text(label),
      trailing: Text(value, style: Theme.of(context).textTheme.bodyMedium),
    );
  }
}
