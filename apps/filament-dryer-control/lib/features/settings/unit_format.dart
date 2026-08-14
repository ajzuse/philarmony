/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import '../../data/app_preferences.dart';

String formatTemperature(double? celsius, {required bool useCelsius}) {
  if (celsius == null) return '—';
  if (useCelsius) return '${celsius.toStringAsFixed(1)} °C';
  final f = celsius * 9 / 5 + 32;
  return '${f.toStringAsFixed(1)} °F';
}

String formatHumidity(double? pct) {
  if (pct == null) return '—';
  return '${pct.toStringAsFixed(1)} %RH';
}

String formatDuration(int totalSeconds, {required AppTimeFormat timeFormat}) {
  if (totalSeconds < 0) return '—';
  final hours = totalSeconds ~/ 3600;
  final minutes = (totalSeconds % 3600) ~/ 60;
  final seconds = totalSeconds % 60;

  if (timeFormat == AppTimeFormat.h24) {
    if (hours > 0) {
      return '${hours.toString().padLeft(2, '0')}:'
          '${minutes.toString().padLeft(2, '0')}:'
          '${seconds.toString().padLeft(2, '0')}';
    }
    return '${minutes.toString().padLeft(2, '0')}:'
        '${seconds.toString().padLeft(2, '0')}';
  }

  final period = hours >= 12 ? 'PM' : 'AM';
  final h12 = hours % 12 == 0 ? 12 : hours % 12;
  if (hours > 0 || minutes > 0) {
    return '$h12:${minutes.toString().padLeft(2, '0')}:'
        '${seconds.toString().padLeft(2, '0')} $period';
  }
  return '0:${seconds.toString().padLeft(2, '0')} $period';
}

String formatPercent(double? value) {
  if (value == null) return '—';
  return '${value.toStringAsFixed(0)} %';
}

String formatBytes(int? bytes) {
  if (bytes == null) return '—';
  if (bytes >= 1024 * 1024) {
    return '${(bytes / (1024 * 1024)).toStringAsFixed(1)} MB';
  }
  if (bytes >= 1024) {
    return '${(bytes / 1024).toStringAsFixed(0)} KB';
  }
  return '$bytes B';
}

String formatOnOff(bool on) => on ? 'On' : 'Off';
