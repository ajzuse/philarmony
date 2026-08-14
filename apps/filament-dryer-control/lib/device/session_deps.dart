/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shared_preferences/shared_preferences.dart';

import '../data/app_database.dart';
import '../data/app_preferences.dart';
import '../data/drying_cycle_repository.dart';
import '../data/known_device_repository.dart';
import '../data/pending_command_repository.dart';
import 'device_interfaces.dart';
import 'fake_philarmony_ws_client.dart';
import 'philarmony_ws_client.dart';
import 'telemetry_buffer.dart';

const useFakeWs = bool.fromEnvironment('USE_FAKE_WS', defaultValue: true);

final sharedPreferencesProvider = FutureProvider<SharedPreferences>((ref) async {
  return SharedPreferences.getInstance();
});

final appDatabaseProvider = Provider<AppDatabase>((ref) {
  final db = AppDatabase();
  ref.onDispose(db.close);
  return db;
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
  final store = await ref.watch(localStoreProvider.future);
  return KnownDeviceRepository(store);
});

final dryingCycleRepositoryProvider = FutureProvider<DryingCycleRepository>((ref) async {
  final store = await ref.watch(localStoreProvider.future);
  return DryingCycleRepository(store);
});

final pendingCommandRepositoryProvider =
    FutureProvider<PendingCommandRepository>((ref) async {
  final store = await ref.watch(localStoreProvider.future);
  return PendingCommandRepository(store);
});

final wsClientFactoryProvider = Provider<PhilarmonyWsClient Function()>((ref) {
  return () => useFakeWs ? FakePhilarmonyWsClient() : PhilarmonyWsClientImpl();
});

final telemetryBufferProvider = Provider<TelemetryBuffer>((ref) {
  final buffer = TelemetryBuffer();
  ref.onDispose(buffer.clear);
  return buffer;
});
