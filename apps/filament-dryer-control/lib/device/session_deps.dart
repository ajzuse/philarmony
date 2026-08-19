/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:io' show Platform;

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shared_preferences/shared_preferences.dart';

import '../data/app_database.dart';
import '../data/app_preferences.dart';
import '../data/drying_cycle_repository.dart';
import '../data/known_device_repository.dart';
import '../data/pending_command_repository.dart';
import '../data/store_migration.dart';
import 'device_interfaces.dart';
import 'fake_philarmony_ws_client.dart';
import 'last_valid_readings.dart';
import 'philarmony_ws_client.dart';
import 'telemetry_buffer.dart';

const useFakeWs = bool.fromEnvironment('USE_FAKE_WS', defaultValue: false);

final sharedPreferencesProvider = FutureProvider<SharedPreferences>((ref) async {
  return SharedPreferences.getInstance();
});

final appDatabaseProvider = Provider<AppDatabase>((ref) {
  final inTest = Platform.environment.containsKey('FLUTTER_TEST');
  final db = inTest ? AppDatabase.memory() : AppDatabase();
  ref.onDispose(db.close);
  return db;
});

final storeMigrationProvider = FutureProvider<void>((ref) async {
  final prefs = await ref.watch(sharedPreferencesProvider.future);
  final db = ref.watch(appDatabaseProvider);
  await migrateStoreIfNeeded(prefs, db);
});

final localStoreProvider = FutureProvider<LocalStore>((ref) async {
  final prefs = await ref.watch(sharedPreferencesProvider.future);
  return LocalStore(prefs);
});

final appPreferencesProvider = FutureProvider<AppPreferences>((ref) async {
  final prefs = await ref.watch(sharedPreferencesProvider.future);
  return AppPreferences(prefs);
});

final knownDeviceRepositoryProvider = FutureProvider<KnownDeviceRepository>((ref) async {
  await ref.watch(storeMigrationProvider.future);
  final db = ref.watch(appDatabaseProvider);
  return KnownDeviceRepository(db);
});

final dryingCycleRepositoryProvider = FutureProvider<DryingCycleRepository>((ref) async {
  await ref.watch(storeMigrationProvider.future);
  final db = ref.watch(appDatabaseProvider);
  return DryingCycleRepository(db);
});

final pendingCommandRepositoryProvider =
    FutureProvider<PendingCommandRepository>((ref) async {
  await ref.watch(storeMigrationProvider.future);
  final db = ref.watch(appDatabaseProvider);
  return PendingCommandRepository(db);
});

final wsClientFactoryProvider = Provider<PhilarmonyWsClient Function()>((ref) {
  return () => useFakeWs ? FakePhilarmonyWsClient() : PhilarmonyWsClientImpl();
});

final telemetryBufferProvider = Provider<TelemetryBuffer>((ref) {
  final buffer = TelemetryBuffer();
  ref.onDispose(buffer.clear);
  return buffer;
});

final lastValidReadingsProvider = Provider<LastValidReadings>((ref) {
  return LastValidReadings();
});
