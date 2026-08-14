/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:filament_dryer_control/shell/adaptive_scaffold.dart';

void main() {
  testWidgets('AdaptiveScaffold uses bottom nav on narrow width', (tester) async {
    await tester.pumpWidget(
      MaterialApp(
        home: MediaQuery(
          data: const MediaQueryData(size: Size(400, 800)),
          child: Scaffold(
            body: AdaptiveScaffold(
              selectedIndex: 0,
              onDestinationSelected: (_) {},
              destinations: const [
                NavigationDestination(icon: Icon(Icons.home), label: 'Home'),
                NavigationDestination(icon: Icon(Icons.settings), label: 'Settings'),
              ],
              body: const Text('body'),
            ),
          ),
        ),
      ),
    );
    expect(find.byType(NavigationBar), findsOneWidget);
  });

  testWidgets('AdaptiveScaffold uses rail on wide width', (tester) async {
    await tester.pumpWidget(
      MaterialApp(
        home: MediaQuery(
          data: const MediaQueryData(size: Size(900, 800)),
          child: Scaffold(
            body: AdaptiveScaffold(
              selectedIndex: 0,
              onDestinationSelected: (_) {},
              destinations: const [
                NavigationDestination(icon: Icon(Icons.home), label: 'Home'),
                NavigationDestination(icon: Icon(Icons.settings), label: 'Settings'),
              ],
              body: const Text('body'),
            ),
          ),
        ),
      ),
    );
    expect(find.byType(NavigationRail), findsOneWidget);
  });
}
