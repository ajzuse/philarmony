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

/// Read-only view of the active device session (mirrors [sessionManagerProvider]).
final deviceSessionProvider = Provider<DeviceSessionState>((ref) {
  return ref.watch(sessionManagerProvider).activeDeviceSessionState;
});

/// Session actions (connect, disconnect, cycle control).
final deviceSessionActionsProvider = Provider<SessionManagerNotifier>((ref) {
  return ref.read(sessionManagerProvider.notifier);
});

final knownDevicesProvider = FutureProvider<List<KnownDevice>>((ref) async {
  final repo = await ref.read(knownDeviceRepositoryProvider.future);
  return repo.list();
});
