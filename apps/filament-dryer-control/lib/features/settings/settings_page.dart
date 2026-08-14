/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../../data/app_preferences.dart';
import '../../device/session_deps.dart';
import '../../l10n/app_localizations.dart';
import '../../platform/local_notifications.dart';

class SettingsPage extends ConsumerWidget {
  const SettingsPage({super.key});

  String _languageLabel(AppLocalizations l10n, String? code) {
    return switch (code) {
      'en_US' => l10n.languageEnUs,
      'pt_BR' => l10n.languagePtBr,
      _ => l10n.systemDefault,
    };
  }

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
            subtitle: Text(_languageLabel(l10n, prefs.localeCode)),
            trailing: DropdownButton<String?>(
              value: prefs.localeCode,
              underline: const SizedBox.shrink(),
              items: [
                DropdownMenuItem(
                  value: null,
                  child: Text(l10n.systemDefault),
                ),
                DropdownMenuItem(
                  value: 'en_US',
                  child: Text(l10n.languageEnUs),
                ),
                DropdownMenuItem(
                  value: 'pt_BR',
                  child: Text(l10n.languagePtBr),
                ),
              ],
              onChanged: (value) async {
                await prefs.setLocaleCode(value);
                ref.invalidate(appPreferencesProvider);
              },
            ),
          ),
          ListTile(
            title: Text(l10n.materialProfiles),
            subtitle: Text(l10n.materialProfilesSubtitle),
            trailing: const Icon(Icons.chevron_right),
            onTap: () => context.push('/profiles'),
          ),
          SwitchListTile(
            title: Text(l10n.useCelsius),
            subtitle: Text(l10n.useCelsiusSubtitle),
            value: prefs.useCelsius,
            onChanged: (v) async {
              await prefs.setUseCelsius(v);
              ref.invalidate(appPreferencesProvider);
            },
          ),
          SwitchListTile(
            title: Text(l10n.highContrast),
            subtitle: Text(l10n.highContrastSubtitle),
            value: prefs.highContrast,
            onChanged: (v) async {
              await prefs.setHighContrast(v);
              ref.invalidate(appPreferencesProvider);
            },
          ),
          ListTile(
            title: Text(l10n.timeFormat),
            subtitle: Text(
              prefs.timeFormat == AppTimeFormat.h24
                  ? l10n.timeFormat24h
                  : l10n.timeFormat12h,
            ),
            trailing: DropdownButton<AppTimeFormat>(
              value: prefs.timeFormat,
              underline: const SizedBox.shrink(),
              items: [
                DropdownMenuItem(
                  value: AppTimeFormat.h24,
                  child: Text(l10n.timeFormat24h),
                ),
                DropdownMenuItem(
                  value: AppTimeFormat.h12,
                  child: Text(l10n.timeFormat12h),
                ),
              ],
              onChanged: (value) async {
                if (value == null) return;
                await prefs.setTimeFormat(value);
                ref.invalidate(appPreferencesProvider);
              },
            ),
          ),
          ListTile(
            title: Text(l10n.chartWindow),
            subtitle: Text(l10n.chartWindowSubtitle),
            trailing: DropdownButton<int>(
              value: prefs.chartWindowMinutes,
              underline: const SizedBox.shrink(),
              items: const [15, 30, 60, 120]
                  .map(
                    (m) => DropdownMenuItem(
                      value: m,
                      child: Text(l10n.chartWindowMinutes(m)),
                    ),
                  )
                  .toList(),
              onChanged: (value) async {
                if (value == null) return;
                await prefs.setChartWindowMinutes(value);
                ref.invalidate(appPreferencesProvider);
              },
            ),
          ),
          ListTile(
            title: Text(l10n.maxConcurrentSessions),
            subtitle: Text(l10n.maxConcurrentSessionsSubtitle),
            trailing: DropdownButton<int>(
              value: prefs.maxConcurrentSessions,
              underline: const SizedBox.shrink(),
              items: const [1, 2, 3, 4, 5]
                  .map(
                    (n) => DropdownMenuItem(
                      value: n,
                      child: Text('$n'),
                    ),
                  )
                  .toList(),
              onChanged: (value) async {
                if (value == null) return;
                await prefs.setMaxConcurrentSessions(value);
                ref.invalidate(appPreferencesProvider);
              },
            ),
          ),
          ListTile(
            title: Text(l10n.pinLock),
            subtitle: Text(l10n.pinLockSubtitle),
          ),
          ListTile(
            title: Text(l10n.deviceConfig),
            subtitle: Text(l10n.deviceConfigSubtitle),
            trailing: const Icon(Icons.chevron_right),
            onTap: () => context.push('/config'),
          ),
          ListTile(
            title: Text(l10n.testNotification),
            subtitle: Text(l10n.testNotificationSubtitle),
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
