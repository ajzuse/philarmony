/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../../device/session_providers.dart';
import '../../l10n/app_localizations.dart';
import '../../platform/local_notifications.dart';

class SettingsPage extends ConsumerWidget {
  const SettingsPage({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final prefsAsync = ref.watch(appPreferencesProvider);

    return prefsAsync.when(
      data: (prefs) => ListView(
        padding: const EdgeInsets.all(16),
        children: [
          ListTile(
            title: Text(l10n.language),
            subtitle: Text(prefs.localeCode ?? 'System default'),
          ),
          ListTile(
            title: Text(l10n.materialProfiles),
            subtitle: Text(l10n.materialProfilesSubtitle),
            trailing: const Icon(Icons.chevron_right),
            onTap: () => context.push('/profiles'),
          ),
          SwitchListTile(
            title: const Text('Use Celsius'),
            subtitle: const Text('Off shows Fahrenheit on dashboard'),
            value: prefs.useCelsius,
            onChanged: (v) async {
              await prefs.setUseCelsius(v);
              ref.invalidate(appPreferencesProvider);
            },
          ),
          ListTile(
            title: const Text('Max concurrent sessions'),
            subtitle: Text('${prefs.maxConcurrentSessions}'),
          ),
          const ListTile(
            title: Text('PIN lock'),
            subtitle: Text('Optional — stub for MVP'),
          ),
          ListTile(
            title: const Text('Device configuration'),
            subtitle: const Text('Sensors, pins, display, WiFi'),
            trailing: const Icon(Icons.chevron_right),
            onTap: () => context.push('/config'),
          ),
          ListTile(
            title: const Text('Test notification'),
            subtitle: const Text('Local notifications adapter stub'),
            onTap: () => ref.read(localNotificationsProvider).show(
                  title: 'Philarmony',
                  body: 'Test notification',
                ),
          ),
        ],
      ),
      loading: () => const Center(child: CircularProgressIndicator()),
      error: (e, _) => Center(child: Text('$e')),
    );
  }
}
