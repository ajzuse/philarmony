/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../device/session_providers.dart';

class WifiSection extends ConsumerWidget {
  const WifiSection({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final device = ref.watch(deviceSessionProvider).activeDevice;
    final theme = Theme.of(context);

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('WiFi', style: theme.textTheme.titleMedium),
            const SizedBox(height: 8),
            ListTile(
              contentPadding: EdgeInsets.zero,
              title: const Text('SSID'),
              subtitle: Text(device?.host ?? 'Not connected'),
            ),
            ListTile(
              contentPadding: EdgeInsets.zero,
              title: const Text('Signal'),
              subtitle: const Text('Not reported by device (MVP)'),
            ),
            const SizedBox(height: 8),
            OutlinedButton.icon(
              onPressed: null,
              icon: const Icon(Icons.wifi_tethering),
              label: const Text('Reconfigure WiFi (hotspot mode)'),
            ),
            Text(
              'WiFi reconfiguration triggers hotspot mode on the device. '
              'Use the desktop installer for full WiFi setup in v1.',
              style: theme.textTheme.bodySmall,
            ),
          ],
        ),
      ),
    );
  }
}
