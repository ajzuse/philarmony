/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

String formatTemperature(double? celsius, {required bool useCelsius}) {
  if (celsius == null) return '—';
  if (useCelsius) return '${celsius.toStringAsFixed(1)} °C';
  final f = celsius * 9 / 5 + 32;
  return '${f.toStringAsFixed(1)} °F';
}

String formatHumidity(double? pct) {
  if (pct == null) return '—';
  return '${pct.toStringAsFixed(1)} %';
}
