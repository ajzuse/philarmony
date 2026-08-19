/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../device/session_providers.dart';
import '../l10n/app_localizations.dart';

class DeviceSwitcher extends ConsumerWidget {
  const DeviceSwitcher({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final manager = ref.watch(sessionManagerProvider);
    final sessions = manager.sessions.values.toList();

    if (sessions.isEmpty) {
      return const SizedBox.shrink();
    }

    final activeId = manager.activeDeviceId;
    final active = manager.activeSession;

    return PopupMenuButton<String>(
      tooltip: l10n.switchDevice,
      onSelected: (deviceId) {
        ref.read(deviceSessionActionsProvider).setActive(deviceId);
      },
      itemBuilder: (context) {
        return [
          ...sessions.map((entry) {
            final device = entry.device;
            final selected = device.id == activeId;
            return PopupMenuItem<String>(
              value: device.id,
              child: Row(
                children: [
                  if (selected)
                    const Icon(Icons.check, size: 18)
                  else
                    const SizedBox(width: 18),
                  const SizedBox(width: 8),
                  Expanded(child: Text(device.nickname)),
                ],
              ),
            );
          }),
          const PopupMenuDivider(),
          PopupMenuItem<String>(
            enabled: false,
            child: Text(
              l10n.sessionCount(sessions.length, manager.maxConcurrentSessions),
              style: Theme.of(context).textTheme.bodySmall,
            ),
          ),
        ];
      },
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 8),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Icon(Icons.devices, size: 20),
            const SizedBox(width: 4),
            Text(
              active?.device.nickname ?? l10n.navDevices,
              style: Theme.of(context).textTheme.titleSmall,
            ),
            const Icon(Icons.arrow_drop_down),
          ],
        ),
      ),
    );
  }
}

/// Compact chip showing the active device nickname for tests and narrow layouts.
class ActiveDeviceChip extends ConsumerWidget {
  const ActiveDeviceChip({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final device = ref.watch(deviceSessionProvider).activeDevice;
    if (device == null) return const SizedBox.shrink();
    return Chip(
      avatar: const Icon(Icons.devices, size: 16),
      label: Text(device.nickname),
    );
  }
}
