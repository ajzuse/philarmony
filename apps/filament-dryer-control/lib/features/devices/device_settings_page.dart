/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../device/session_providers.dart';
import '../../l10n/app_localizations.dart';

/// Per-device notification event keys stored in [KnownDevice.notificationSettings].
abstract final class DeviceNotificationEvents {
  static const cycleComplete = 'cycle_complete';
  static const cycleError = 'cycle_error';
  static const safetyFault = 'safety_fault';
  static const connectionLost = 'connection_lost';

  static const all = [
    cycleComplete,
    cycleError,
    safetyFault,
    connectionLost,
  ];
}

class DeviceSettingsPage extends ConsumerWidget {
  const DeviceSettingsPage({super.key, required this.deviceId});

  final String deviceId;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final knownAsync = ref.watch(knownDevicesProvider);

    return Scaffold(
      appBar: AppBar(title: Text(l10n.deviceSettings)),
      body: knownAsync.when(
        data: (devices) {
          KnownDevice? device;
          for (final d in devices) {
            if (d.id == deviceId) {
              device = d;
              break;
            }
          }
          if (device == null) {
            return Center(child: Text(l10n.deviceNotFound));
          }
          final knownDevice = device;
          return ListView(
            padding: const EdgeInsets.all(16),
            children: [
              ListTile(
                title: Text(knownDevice.nickname),
                subtitle: Text(knownDevice.wsUri),
              ),
              const Divider(),
              Text(
                l10n.notificationSettings,
                style: Theme.of(context).textTheme.titleMedium,
              ),
              ...DeviceNotificationEvents.all.map((eventKey) {
                final enabled =
                    knownDevice.notificationSettings[eventKey] ?? true;
                return SwitchListTile(
                  title: Text(_labelForEvent(l10n, eventKey)),
                  value: enabled,
                  onChanged: (value) async {
                    final updated = Map<String, bool>.from(
                      knownDevice.notificationSettings,
                    )..[eventKey] = value;
                    final next = knownDevice.copyWith(
                      notificationSettings: updated,
                    );
                    final repo =
                        await ref.read(knownDeviceRepositoryProvider.future);
                    await repo.upsert(next);
                    ref.invalidate(knownDevicesProvider);
                  },
                );
              }),
            ],
          );
        },
        loading: () => const Center(child: CircularProgressIndicator()),
        error: (e, _) => Center(child: Text('$e')),
      ),
    );
  }

  String _labelForEvent(AppLocalizations l10n, String eventKey) {
    return switch (eventKey) {
      DeviceNotificationEvents.cycleComplete => l10n.notifyCycleComplete,
      DeviceNotificationEvents.cycleError => l10n.notifyCycleError,
      DeviceNotificationEvents.safetyFault => l10n.notifySafetyFault,
      DeviceNotificationEvents.connectionLost => l10n.notifyConnectionLost,
      _ => eventKey,
    };
  }
}
