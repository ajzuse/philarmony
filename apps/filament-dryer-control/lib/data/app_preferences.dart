/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

class AppPreferences {
  AppPreferences(this._prefs);

  final SharedPreferences _prefs;

  static const _localeKey = 'locale';
  static const _celsiusKey = 'use_celsius';
  static const _maxSessionsKey = 'max_sessions';

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
}

class LocalStore {
  LocalStore(this._prefs);

  final SharedPreferences _prefs;

  static const _devicesKey = 'known_devices';
  static const _cyclesKey = 'drying_cycles';
  static const _pendingCommandsKey = 'pending_commands';
  static const _profilesCacheKey = 'profiles_cache';

  List<KnownDevice> loadDevices() {
    final raw = _prefs.getString(_devicesKey);
    if (raw == null) return [];
    final list = jsonDecode(raw) as List<dynamic>;
    return list
        .map((e) => KnownDevice.fromJson(Map<String, dynamic>.from(e as Map)))
        .toList();
  }

  Future<void> saveDevices(List<KnownDevice> devices) async {
    final json = jsonEncode(devices.map((d) => d.toJson()).toList());
    await _prefs.setString(_devicesKey, json);
  }

  List<Map<String, dynamic>> loadCycles() {
    final raw = _prefs.getString(_cyclesKey);
    if (raw == null) return [];
    return (jsonDecode(raw) as List<dynamic>)
        .map((e) => Map<String, dynamic>.from(e as Map))
        .toList();
  }

  Future<void> saveCycles(List<Map<String, dynamic>> cycles) async {
    await _prefs.setString(_cyclesKey, jsonEncode(cycles));
  }

  List<Map<String, dynamic>> loadPendingCommands() {
    final raw = _prefs.getString(_pendingCommandsKey);
    if (raw == null) return [];
    return (jsonDecode(raw) as List<dynamic>)
        .map((e) => Map<String, dynamic>.from(e as Map))
        .toList();
  }

  Future<void> savePendingCommands(List<Map<String, dynamic>> commands) async {
    await _prefs.setString(_pendingCommandsKey, jsonEncode(commands));
  }

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
}
