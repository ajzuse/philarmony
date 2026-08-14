/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';

import 'app_breakpoint.dart';

class AdaptiveScaffold extends StatelessWidget {
  const AdaptiveScaffold({
    super.key,
    required this.selectedIndex,
    required this.onDestinationSelected,
    required this.destinations,
    required this.body,
    this.detail,
    this.appBar,
  });

  final int selectedIndex;
  final ValueChanged<int> onDestinationSelected;
  final List<NavigationDestination> destinations;
  final Widget body;
  final Widget? detail;
  final PreferredSizeWidget? appBar;

  @override
  Widget build(BuildContext context) {
    final bp = appBreakpointOf(context);
    if (bp.useBottomNav) {
      return Scaffold(
        appBar: appBar,
        body: body,
        bottomNavigationBar: NavigationBar(
          selectedIndex: selectedIndex,
          onDestinationSelected: onDestinationSelected,
          destinations: destinations,
        ),
      );
    }

    final railDestinations = destinations
        .map((d) => NavigationRailDestination(
              icon: d.icon,
              selectedIcon: d.selectedIcon ?? d.icon,
              label: Text(d.label),
            ))
        .toList();

    if (bp.useMasterDetail && detail != null) {
      return Scaffold(
        appBar: appBar,
        body: Row(
          children: [
            NavigationRail(
              selectedIndex: selectedIndex,
              onDestinationSelected: onDestinationSelected,
              labelType: NavigationRailLabelType.all,
              destinations: railDestinations,
            ),
            const VerticalDivider(width: 1),
            Expanded(flex: 2, child: body),
            const VerticalDivider(width: 1),
            Expanded(flex: 3, child: detail!),
          ],
        ),
      );
    }

    return Scaffold(
      appBar: appBar,
      body: Row(
        children: [
          NavigationRail(
            selectedIndex: selectedIndex,
            onDestinationSelected: onDestinationSelected,
            labelType: NavigationRailLabelType.all,
            destinations: railDestinations,
          ),
          const VerticalDivider(width: 1),
          Expanded(child: body),
        ],
      ),
    );
  }
}
