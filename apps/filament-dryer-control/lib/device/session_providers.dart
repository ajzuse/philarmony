/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import 'device_interfaces.dart';
import 'session_deps.dart';
import 'session_manager.dart';
import 'session_state.dart';

export 'session_deps.dart';
export 'session_manager.dart';
export 'session_state.dart';

/// Active session WebSocket client (for tests that override a single fake).
final wsClientProvider = Provider<PhilarmonyWsClient>((ref) {
  final active = ref.watch(sessionManagerProvider).activeSession;
  if (active != null) return active.client;
  return ref.read(wsClientFactoryProvider)();
});

class DeviceSessionNotifier extends Notifier<DeviceSessionState> {
  @override
  DeviceSessionState build() {
    ref.listen(sessionManagerProvider, (_, next) {
      state = next.activeDeviceSessionState;
    }, fireImmediately: true);
    return ref.read(sessionManagerProvider).activeDeviceSessionState;
  }

  Future<void> connect(KnownDevice device) =>
      ref.read(sessionManagerProvider.notifier).connect(device);

  Future<void> disconnect([String? deviceId]) =>
      ref.read(sessionManagerProvider.notifier).disconnect(deviceId);

  Future<void> setActive(String deviceId) =>
      ref.read(sessionManagerProvider.notifier).setActive(deviceId);

  Future<void> startCycle(StartCycleRequest request) =>
      ref.read(sessionManagerProvider.notifier).startCycle(request);

  Future<void> stopCycle() =>
      ref.read(sessionManagerProvider.notifier).stopCycle();
}

final deviceSessionProvider =
    NotifierProvider<DeviceSessionNotifier, DeviceSessionState>(
  DeviceSessionNotifier.new,
);

final knownDevicesProvider = FutureProvider<List<KnownDevice>>((ref) async {
  final repo = await ref.read(knownDeviceRepositoryProvider.future);
  return repo.list();
});
