/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';

const _seedColor = Color(0xFF1B5E4B);

ThemeData buildAppTheme() {
  return ThemeData(
    colorScheme: ColorScheme.fromSeed(seedColor: _seedColor),
    useMaterial3: true,
  );
}

ThemeData buildHighContrastTheme() {
  final base = buildAppTheme();
  return base.copyWith(
    colorScheme: base.colorScheme.copyWith(
      primary: const Color(0xFF004D40),
      onPrimary: Colors.white,
      secondary: const Color(0xFF00695C),
      onSecondary: Colors.white,
      surface: Colors.white,
      onSurface: Colors.black,
      outline: Colors.black,
    ),
    dividerColor: Colors.black,
    visualDensity: VisualDensity.standard,
  );
}
