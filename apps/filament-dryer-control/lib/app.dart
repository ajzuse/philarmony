/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'features/discover/discover_controller.dart';
import 'l10n/app_localizations.dart';
import 'shell/app_router.dart';
import 'theme/app_theme.dart';

class PhilarmonyControlApp extends ConsumerWidget {
  const PhilarmonyControlApp({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    ref.watch(discoverBootstrapProvider);
    final router = ref.watch(appRouterProvider);
    return MaterialApp.router(
      title: 'Philarmony Control',
      debugShowCheckedModeBanner: false,
      theme: buildAppTheme(),
      localizationsDelegates: const [
        AppLocalizations.delegate,
        GlobalMaterialLocalizations.delegate,
        GlobalWidgetsLocalizations.delegate,
        GlobalCupertinoLocalizations.delegate,
      ],
      supportedLocales: AppLocalizations.supportedLocales,
      routerConfig: router,
    );
  }
}
