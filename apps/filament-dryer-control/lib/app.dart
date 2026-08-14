/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'device/session_deps.dart';
import 'features/discover/discover_controller.dart';
import 'l10n/app_localizations.dart';
import 'shell/app_messenger.dart';
import 'shell/app_router.dart';
import 'theme/app_theme.dart';

Locale? localeFromPreferenceCode(String? code) {
  return switch (code) {
    'pt_BR' => const Locale('pt', 'BR'),
    'en_US' => const Locale('en', 'US'),
    _ => null,
  };
}

class PhilarmonyControlApp extends ConsumerWidget {
  const PhilarmonyControlApp({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    ref.watch(discoverBootstrapProvider);
    final router = ref.watch(appRouterProvider);
    final prefs = ref.watch(appPreferencesProvider).valueOrNull;
    final locale = localeFromPreferenceCode(prefs?.localeCode);
    final highContrastPref = prefs?.highContrast ?? false;

    return MaterialApp.router(
      title: 'Philarmony Control',
      debugShowCheckedModeBanner: false,
      scaffoldMessengerKey: ref.watch(scaffoldMessengerKeyProvider),
      theme: buildAppTheme(),
      highContrastTheme: buildHighContrastTheme(),
      themeMode: ThemeMode.system,
      locale: locale,
      localizationsDelegates: const [
        AppLocalizations.delegate,
        GlobalMaterialLocalizations.delegate,
        GlobalWidgetsLocalizations.delegate,
        GlobalCupertinoLocalizations.delegate,
      ],
      supportedLocales: AppLocalizations.supportedLocales,
      builder: (context, child) {
        final systemHighContrast = MediaQuery.highContrastOf(context);
        final useHighContrast = systemHighContrast || highContrastPref;
        final textDirection =
            Directionality.maybeOf(context) ?? TextDirection.ltr;
        Widget content = child ?? const SizedBox.shrink();
        if (useHighContrast && !systemHighContrast) {
          content = Theme(
            data: buildHighContrastTheme(),
            child: content,
          );
        }
        return Directionality(
          textDirection: textDirection,
          child: content,
        );
      },
      routerConfig: router,
    );
  }
}
