/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../features/cycle/cycle_result_page.dart';
import '../features/cycle/start_cycle_page.dart';
import '../features/config/config_page.dart';
import '../features/dashboard/dashboard_page.dart';
import '../features/discover/discover_page.dart';
import '../features/history/cycle_detail_page.dart';
import '../features/history/history_controller.dart';
import '../features/history/history_page.dart';
import '../features/profiles/profiles_page.dart';
import '../features/devices/device_settings_page.dart';
import '../features/settings/settings_page.dart';
import '../l10n/app_localizations.dart';
import '../shell/adaptive_scaffold.dart';
import '../shell/device_switcher.dart';

final _rootNavigatorKey = GlobalKey<NavigatorState>();

final appRouterProvider = Provider<GoRouter>((ref) {
  return GoRouter(
    navigatorKey: _rootNavigatorKey,
    initialLocation: '/dashboard',
    routes: [
      StatefulShellRoute.indexedStack(
        builder: (context, state, navigationShell) => AppShellScaffold(
          navigationShell: navigationShell,
        ),
        branches: [
          StatefulShellBranch(
            routes: [
              GoRoute(
                path: '/dashboard',
                builder: (_, __) => const DashboardPage(),
              ),
            ],
          ),
          StatefulShellBranch(
            routes: [
              GoRoute(
                path: '/history',
                builder: (_, __) => const HistoryPage(),
              ),
            ],
          ),
          StatefulShellBranch(
            routes: [
              GoRoute(
                path: '/devices',
                builder: (_, __) => const DiscoverPage(),
              ),
            ],
          ),
          StatefulShellBranch(
            routes: [
              GoRoute(
                path: '/settings',
                builder: (_, __) => const SettingsPage(),
              ),
            ],
          ),
        ],
      ),
      GoRoute(
        parentNavigatorKey: _rootNavigatorKey,
        path: '/config',
        builder: (_, __) => const ConfigPage(),
      ),
      GoRoute(
        parentNavigatorKey: _rootNavigatorKey,
        path: '/device-settings/:id',
        builder: (_, state) {
          final id = state.pathParameters['id']!;
          return DeviceSettingsPage(deviceId: id);
        },
      ),
      GoRoute(
        parentNavigatorKey: _rootNavigatorKey,
        path: '/start-cycle',
        builder: (_, __) => const StartCyclePage(),
      ),
      GoRoute(
        parentNavigatorKey: _rootNavigatorKey,
        path: '/profiles',
        builder: (_, __) => const ProfilesPage(),
      ),
      GoRoute(
        parentNavigatorKey: _rootNavigatorKey,
        path: '/cycle-result',
        builder: (context, state) {
          final extra = state.extra;
          if (extra is! Map<String, dynamic>) {
            return const Scaffold(body: Center(child: Text('Cycle not found')));
          }
          return CycleResultPage(cycle: extra);
        },
      ),
    ],
  );
});

class AppShellScaffold extends ConsumerWidget {
  const AppShellScaffold({super.key, required this.navigationShell});

  final StatefulNavigationShell navigationShell;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final destinations = [
      NavigationDestination(
        icon: const Icon(Icons.dashboard_outlined),
        selectedIcon: const Icon(Icons.dashboard),
        label: l10n.navDashboard,
      ),
      NavigationDestination(
        icon: const Icon(Icons.history_outlined),
        selectedIcon: const Icon(Icons.history),
        label: l10n.navHistory,
      ),
      NavigationDestination(
        icon: const Icon(Icons.devices_outlined),
        selectedIcon: const Icon(Icons.devices),
        label: l10n.navDevices,
      ),
      NavigationDestination(
        icon: const Icon(Icons.settings_outlined),
        selectedIcon: const Icon(Icons.settings),
        label: l10n.navSettings,
      ),
    ];
    final detail = switch (navigationShell.currentIndex) {
      0 => const DashboardChartsPane(),
      1 => HistoryDetailPane(cycle: ref.watch(selectedCycleProvider)),
      _ => null,
    };
    return AdaptiveScaffold(
      selectedIndex: navigationShell.currentIndex,
      onDestinationSelected: navigationShell.goBranch,
      destinations: destinations,
      appBar: AppBar(
        title: Text(l10n.appTitle),
        actions: const [DeviceSwitcher()],
      ),
      body: navigationShell,
      detail: detail,
    );
  }
}
