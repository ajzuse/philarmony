/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../device/session_providers.dart';
import 'config_controller.dart';

class AdvancedSection extends ConsumerWidget {
  const AdvancedSection({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final config = ref.watch(configControllerProvider).draft;
    final status = ref.watch(deviceSessionProvider).status;
    final device = ref.watch(deviceSessionProvider).activeDevice;
    final theme = Theme.of(context);

    final safety = config.control['safety_limits'] as Map<String, dynamic>? ?? {};
    final hardLimit = safety['hard_temp_limit_c'];

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Advanced', style: theme.textTheme.titleMedium),
            const SizedBox(height: 8),
            _ReadOnlyRow(label: 'Device name', value: device?.nickname ?? '—'),
            _ReadOnlyRow(label: 'Host', value: device?.host ?? '—'),
            _ReadOnlyRow(label: 'Firmware version', value: 'Not reported (MVP)'),
            _ReadOnlyRow(label: 'NTP timezone', value: 'Not reported (MVP)'),
            _ReadOnlyRow(
              label: 'Hard temp limit (°C)',
              value: hardLimit?.toString() ?? '—',
            ),
            _ReadOnlyRow(
              label: 'Uptime (sec)',
              value: status?.uptimeSec.toString() ?? '—',
            ),
            _ReadOnlyRow(
              label: 'CPU usage (%)',
              value: status?.cpuUsagePct?.toStringAsFixed(1) ?? '—',
            ),
            _ReadOnlyRow(
              label: 'Free memory (bytes)',
              value: status?.memoryFreeBytes?.toString() ?? '—',
            ),
          ],
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
