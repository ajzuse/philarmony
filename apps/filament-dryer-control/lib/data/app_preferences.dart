/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

enum AppTimeFormat { h24, h12 }

class AppPreferences {
  AppPreferences(this._prefs);

  final SharedPreferences _prefs;

  static const _localeKey = 'locale';
  static const _celsiusKey = 'use_celsius';
  static const _maxSessionsKey = 'max_sessions';
  static const _timeFormatKey = 'time_format';
  static const _chartWindowKey = 'chart_window_minutes';
  static const _highContrastKey = 'high_contrast';

  String? get localeCode => _prefs.getString(_localeKey);
  Future<void> setLocaleCode(String? code) async {
    if (code == null) {
      await _prefs.remove(_localeKey);
    } else {
      await _prefs.setString(_localeKey, code);
    }
  }

  bool get useCelsius => _prefs.getBool(_celsiusKey) ?? true;
  Future<void> setUseCelsius(bool value) => _prefs.setBool(_celsiusKey, value);

  int get maxConcurrentSessions => _prefs.getInt(_maxSessionsKey) ?? 3;

  Future<void> setMaxConcurrentSessions(int value) =>
      _prefs.setInt(_maxSessionsKey, value.clamp(1, 5));

  AppTimeFormat get timeFormat {
    final raw = _prefs.getString(_timeFormatKey);
    if (raw == 'h12') return AppTimeFormat.h12;
    return AppTimeFormat.h24;
  }

  Future<void> setTimeFormat(AppTimeFormat value) =>
      _prefs.setString(_timeFormatKey, value.name);

  int get chartWindowMinutes => _prefs.getInt(_chartWindowKey) ?? 60;

  Future<void> setChartWindowMinutes(int minutes) =>
      _prefs.setInt(_chartWindowKey, minutes);

  bool get highContrast => _prefs.getBool(_highContrastKey) ?? false;

  Future<void> setHighContrast(bool value) =>
      _prefs.setBool(_highContrastKey, value);
}

/// Profiles cache only — device/cycle/pending data lives in Drift.
class LocalStore {
  LocalStore(this._prefs);

  final SharedPreferences _prefs;

  static const _profilesCacheKey = 'profiles_cache';

  // Legacy keys read once by [store_migration.dart].
  static const legacyDevicesKey = 'known_devices';
  static const legacyCyclesKey = 'drying_cycles';
  static const legacyPendingCommandsKey = 'pending_commands';

  List<Map<String, dynamic>> loadProfilesForDevice(String deviceId) {
    final raw = _prefs.getString(_profilesCacheKey);
    if (raw == null) return [];
    final map = jsonDecode(raw) as Map<String, dynamic>;
    final list = map[deviceId];
    if (list is! List) return [];
    return list
        .map((e) => Map<String, dynamic>.from(e as Map))
        .toList();
  }

  Future<void> saveProfilesForDevice(
    String deviceId,
    List<Map<String, dynamic>> profiles,
  ) async {
    final raw = _prefs.getString(_profilesCacheKey);
    final map = raw == null
        ? <String, dynamic>{}
        : Map<String, dynamic>.from(jsonDecode(raw) as Map);
    map[deviceId] = profiles;
    await _prefs.setString(_profilesCacheKey, jsonEncode(map));
  }

  List<KnownDevice> loadDevices() {
    final raw = _prefs.getString(legacyDevicesKey);
    if (raw == null) return [];
    final list = jsonDecode(raw) as List<dynamic>;
    return list
        .map((e) => KnownDevice.fromJson(Map<String, dynamic>.from(e as Map)))
        .toList();
  }

  List<Map<String, dynamic>> loadCycles() {
    final raw = _prefs.getString(legacyCyclesKey);
    if (raw == null) return [];
    return (jsonDecode(raw) as List<dynamic>)
        .map((e) => Map<String, dynamic>.from(e as Map))
        .toList();
  }

  List<Map<String, dynamic>> loadPendingCommands() {
    final raw = _prefs.getString(legacyPendingCommandsKey);
    if (raw == null) return [];
    return (jsonDecode(raw) as List<dynamic>)
        .map((e) => Map<String, dynamic>.from(e as Map))
        .toList();
  }
}
