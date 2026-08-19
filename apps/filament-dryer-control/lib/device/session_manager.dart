/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';
import 'dart:convert';

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../platform/background_session.dart';
import '../platform/notification_service.dart';
import '../features/cycle/cycle_navigation.dart';
import '../features/history/history_controller.dart';
import '../shell/pending_commands_count.dart';
import '../features/config/config_merge_policy.dart';
import '../features/profiles/profile_sync_service.dart';
import 'device_runtime_info.dart';
import 'device_interfaces.dart';
import 'fake_philarmony_ws_client.dart';
import 'session_deps.dart';
import 'session_state.dart';

/// Raised when [SessionManagerNotifier.connect] would exceed [maxConcurrentSessions].
class SessionLimitExceeded implements Exception {
  SessionLimitExceeded(this.maxSessions);

  final int maxSessions;

  @override
  String toString() =>
      'Maximum concurrent sessions ($maxSessions) reached';
}

/// Runtime state for one connected [KnownDevice].
class DeviceSessionEntry {
  DeviceSessionEntry({
    required this.device,
    required this.client,
    this.connectionState = DeviceConnectionState.idle,
    this.status,
    this.fault,
    this.backgroundAllowed = true,
    this.activeCycleId,
    this.deviceRuntimeInfo,
    this.hardwareCache,
  });

  final KnownDevice device;
  final PhilarmonyWsClient client;
  DeviceConnectionState connectionState;
  StatusSnapshot? status;
  FaultEvent? fault;
  bool backgroundAllowed;
  String? activeCycleId;
  DeviceRuntimeInfo? deviceRuntimeInfo;
  HardwareConfig? hardwareCache;

  DeviceSessionState toDeviceSessionState() {
    return DeviceSessionState(
      activeDevice: device,
      connectionState: connectionState,
      status: status,
      fault: fault,
      backgroundAllowed: backgroundAllowed,
      activeCycleId: activeCycleId,
      deviceRuntimeInfo: deviceRuntimeInfo,
      hardwareCache: hardwareCache,
    );
  }

  DeviceSessionEntry copyWith({
    DeviceConnectionState? connectionState,
    StatusSnapshot? status,
    FaultEvent? fault,
    bool? backgroundAllowed,
    String? activeCycleId,
    bool clearActiveCycleId = false,
    DeviceRuntimeInfo? deviceRuntimeInfo,
    HardwareConfig? hardwareCache,
    KnownDevice? device,
  }) {
    return DeviceSessionEntry(
      device: device ?? this.device,
      client: client,
      connectionState: connectionState ?? this.connectionState,
      status: status ?? this.status,
      fault: fault ?? this.fault,
      backgroundAllowed: backgroundAllowed ?? this.backgroundAllowed,
      activeCycleId: clearActiveCycleId ? null : (activeCycleId ?? this.activeCycleId),
      deviceRuntimeInfo: deviceRuntimeInfo ?? this.deviceRuntimeInfo,
      hardwareCache: hardwareCache ?? this.hardwareCache,
    );
  }
}

class SessionManagerState {
  const SessionManagerState({
    this.sessions = const {},
    this.activeDeviceId,
    this.maxConcurrentSessions = 3,
  });

  final Map<String, DeviceSessionEntry> sessions;
  final String? activeDeviceId;
  final int maxConcurrentSessions;

  DeviceSessionEntry? get activeSession =>
      activeDeviceId != null ? sessions[activeDeviceId] : null;

  DeviceSessionState get activeDeviceSessionState =>
      activeSession?.toDeviceSessionState() ?? const DeviceSessionState();

  DeviceConnectionState connectionStateFor(String deviceId) =>
      sessions[deviceId]?.connectionState ?? DeviceConnectionState.disconnected;

  bool isOnline(String deviceId) {
    final connectionState = sessions[deviceId]?.connectionState;
    return connectionState == DeviceConnectionState.connected ||
        connectionState == DeviceConnectionState.connecting ||
        connectionState == DeviceConnectionState.reconnecting;
  }

  SessionManagerState copyWith({
    Map<String, DeviceSessionEntry>? sessions,
    String? activeDeviceId,
    int? maxConcurrentSessions,
    bool clearActiveDeviceId = false,
  }) {
    return SessionManagerState(
      sessions: sessions ?? this.sessions,
      activeDeviceId:
          clearActiveDeviceId ? null : (activeDeviceId ?? this.activeDeviceId),
      maxConcurrentSessions:
          maxConcurrentSessions ?? this.maxConcurrentSessions,
    );
  }
}

class SessionManagerNotifier extends Notifier<SessionManagerState> {
  final _subscriptions = <String, List<StreamSubscription<dynamic>>>{};

  @override
  SessionManagerState build() {
    ref.listen(appPreferencesProvider, (prev, next) {
      next.whenData((prefs) {
        if (prefs.maxConcurrentSessions != state.maxConcurrentSessions) {
          state = state.copyWith(
            maxConcurrentSessions: prefs.maxConcurrentSessions,
          );
        }
      });
    });
    final maxSessions = ref.read(appPreferencesProvider).maybeWhen(
          data: (prefs) => prefs.maxConcurrentSessions,
          orElse: () => 3,
        );
    ref.onDispose(_disposeAll);
    return SessionManagerState(maxConcurrentSessions: maxSessions);
  }

  Future<void> connect(KnownDevice device) async {
    final existing = state.sessions[device.id];
    if (existing != null) {
      state = state.copyWith(activeDeviceId: device.id);
      if (existing.connectionState != DeviceConnectionState.connected) {
        await existing.client.connect(device);
      }
      _syncEntryConnectionState(device.id, existing.client);
      await _syncDeviceMetadata(_entryFor(device.id));
      await _flushPendingCommands();
      return;
    }

    if (state.sessions.length >= state.maxConcurrentSessions) {
      throw SessionLimitExceeded(state.maxConcurrentSessions);
    }

    final client = ref.read(wsClientFactoryProvider)();
    final entry = DeviceSessionEntry(device: device, client: client);

    final sessions = Map<String, DeviceSessionEntry>.from(state.sessions)
      ..[device.id] = entry;
    state = state.copyWith(sessions: sessions, activeDeviceId: device.id);
    _wireEntry(entry);

    await client.connect(device);
    _syncEntryConnectionState(device.id, client);
    await _syncDeviceMetadata(_entryFor(device.id));
    await _flushPendingCommands();
  }

  void _syncEntryConnectionState(String deviceId, PhilarmonyWsClient client) {
    _updateEntry(
      deviceId,
      _entryFor(deviceId).copyWith(connectionState: client.state),
    );
  }

  Future<void> _syncDeviceMetadata(DeviceSessionEntry entry) async {
    final runtime = entry.client.deviceRuntimeInfo;
    final hardware = entry.client.lastHardwareConfig;
    final now = DateTime.now().toUtc();
    final updatedDevice = entry.device.copyWith(
      lastSeen: now,
      lastConnected: now,
      firmwareVersion: runtime?.firmwareVersion ?? entry.device.firmwareVersion,
      deviceModel: runtime?.deviceModel ?? entry.device.deviceModel,
    );
    final repo = await ref.read(knownDeviceRepositoryProvider.future);
    await repo.upsert(updatedDevice);
    _updateEntry(
      entry.device.id,
      entry.copyWith(
        device: updatedDevice,
        deviceRuntimeInfo: runtime,
        hardwareCache: hardware,
      ),
    );
  }

  Future<void> _flushPendingCommands() async {
    final deviceId = state.activeDeviceId;
    final entry = deviceId != null ? state.sessions[deviceId] : null;
    if (entry == null ||
        entry.connectionState != DeviceConnectionState.connected) {
      return;
    }
    try {
      final store = await ref.read(localStoreProvider.future);
      final pending = await ref.read(pendingCommandRepositoryProvider.future);
      await pending.flush(deviceId!, (cmd) async {
        if (cmd.topic == 'config/hardware' &&
            ConfigMergePolicy.shouldSkipPendingConfig(
              pendingCreatedAt: cmd.createdAt,
              deviceUpdatedAt: entry.client.hardwareConfigUpdatedAt,
            )) {
          return;
        }
        final payload = Map<String, dynamic>.from(
          jsonDecode(cmd.payloadJson) as Map,
        );
        await entry.client.request(
          WsEnvelope(topic: cmd.topic, payload: payload),
        );
      });
      final sync = ProfileSyncService(
        store: store,
        pending: pending,
        getClient: () => entry.client,
        getActiveDeviceId: () => deviceId,
        getConnectionState: () => entry.connectionState,
      );
      await sync.refreshFromDevice();
      ref.read(pendingCommandsCountProvider.notifier).scheduleRefresh();
    } on StateError {
      // Provider container disposed during async flush (tests).
    }
  }

  Future<void> setActive(String deviceId) async {
    if (!state.sessions.containsKey(deviceId)) {
      throw StateError('No session for device $deviceId');
    }
    state = state.copyWith(activeDeviceId: deviceId);
    _syncActiveTelemetry();
  }

  Future<void> disconnect([String? deviceId]) async {
    final id = deviceId ?? state.activeDeviceId;
    if (id == null) return;

    final entry = state.sessions[id];
    if (entry == null) return;

    await _tearDownSession(id, entry);

    final sessions = Map<String, DeviceSessionEntry>.from(state.sessions)
      ..remove(id);
    final wasActive = state.activeDeviceId == id;
    final nextActive = wasActive && sessions.isNotEmpty
        ? sessions.keys.first
        : (wasActive ? null : state.activeDeviceId);
    state = state.copyWith(
      sessions: sessions,
      activeDeviceId: nextActive,
      clearActiveDeviceId: wasActive && sessions.isEmpty,
    );
  }

  Future<void> startCycle(StartCycleRequest request) async {
    final entry = state.activeSession;
    if (entry == null) throw StateError('No active device');

    final repo = await ref.read(dryingCycleRepositoryProvider.future);
    final cycleId = await repo.startCycle(
      deviceId: entry.device.id,
      request: request,
    );
    _updateEntry(
      entry.device.id,
      entry.copyWith(activeCycleId: cycleId),
    );
    await entry.client.startCycle(request);
  }

  Future<void> stopCycle() async {
    final entry = state.activeSession;
    if (entry == null) throw StateError('No active device');

    await entry.client.stopCycle();
    await _finalizeActiveCycle(entry.device.id, 'user_requested');
  }

  void _wireEntry(DeviceSessionEntry entry) {
    final deviceId = entry.device.id;
    _subscriptions[deviceId] = [
      entry.client.connectionStates.listen((connectionState) {
        final prev = _entryFor(deviceId).connectionState;
        final updated = _entryFor(deviceId).copyWith(
          connectionState: connectionState,
        );
        _updateEntry(deviceId, updated);
        unawaited(_notifySession(updated));
        if (connectionState == DeviceConnectionState.connected &&
            prev != DeviceConnectionState.connected) {
          unawaited(_flushPendingCommands());
          _syncRuntimeFromClient(deviceId);
        }
      }),
      entry.client.statusStream.listen((status) {
        _onStatus(deviceId, status);
      }),
      entry.client.faultStream.listen((fault) {
        _updateEntry(deviceId, _entryFor(deviceId).copyWith(fault: fault));
      }),
    ];
  }

  void _syncRuntimeFromClient(String deviceId) {
    final entry = _entryFor(deviceId);
    _updateEntry(
      deviceId,
      entry.copyWith(
        deviceRuntimeInfo: entry.client.deviceRuntimeInfo,
        hardwareCache: entry.client.lastHardwareConfig,
      ),
    );
  }

  DeviceSessionEntry _entryFor(String deviceId) {
    final entry = state.sessions[deviceId];
    if (entry == null) throw StateError('Session $deviceId not found');
    return entry;
  }

  void _updateEntry(String deviceId, DeviceSessionEntry updated) {
    final sessions = Map<String, DeviceSessionEntry>.from(state.sessions)
      ..[deviceId] = updated;
    state = state.copyWith(sessions: sessions);
  }

  void _onStatus(String deviceId, StatusSnapshot status) {
    final entry = _entryFor(deviceId);
    _updateEntry(deviceId, entry.copyWith(status: status));
    _syncRuntimeFromClient(deviceId);

    if (deviceId == state.activeDeviceId) {
      ref.read(telemetryBufferProvider).add(status);
      ref.read(lastValidReadingsProvider).update(status);
    }
    unawaited(_handleStatusSideEffects(deviceId, status));
  }

  Future<void> _handleStatusSideEffects(
    String deviceId,
    StatusSnapshot status,
  ) async {
    final entry = state.sessions[deviceId];
    if (entry == null) return;

    final cycleId = entry.activeCycleId;
    try {
      if (cycleId != null && status.status == DryerStatus.drying) {
        final repo = await ref.read(dryingCycleRepositoryProvider.future);
        await repo.appendSample(cycleId, status);
      }

      if (deviceId != state.activeDeviceId) return;

      final bg = ref.read(backgroundSessionProvider);
      if (status.status == DryerStatus.drying && !bg.isActive) {
        final allowed = await bg.start(reason: 'active_cycle');
        if (!allowed) {
          _updateEntry(
            deviceId,
            _entryFor(deviceId).copyWith(backgroundAllowed: false),
          );
        }
      } else if (status.status != DryerStatus.drying && bg.isActive) {
        await bg.stop();
      }

      if (status.status == DryerStatus.stopped && cycleId != null) {
        await _finalizeActiveCycle(deviceId, 'device_stopped');
      } else if (status.status == DryerStatus.error && cycleId != null) {
        await _finalizeActiveCycle(deviceId, 'safety_cutoff');
      }
    } on StateError {
      // Provider container disposed during async side effects (tests).
    }
  }

  Future<void> _finalizeActiveCycle(String deviceId, String stopReason) async {
    final entry = state.sessions[deviceId];
    if (entry == null) return;
    final cycleId = entry.activeCycleId;
    if (cycleId == null) return;

    final repo = await ref.read(dryingCycleRepositoryProvider.future);
    await repo.finalizeCycle(cycleId, stopReason: stopReason);
    final cycle = await repo.byId(cycleId);
    _updateEntry(deviceId, entry.copyWith(clearActiveCycleId: true));

    if (deviceId == state.activeDeviceId) {
      ref.invalidate(historyCyclesProvider);
      if (cycle != null) {
        ref.read(cycleNavigationProvider.notifier).setPending(cycle);
      }
    }
  }

  void _syncActiveTelemetry() {
    final status = state.activeSession?.status;
    if (status != null) {
      ref.read(telemetryBufferProvider).add(status);
      ref.read(lastValidReadingsProvider).update(status);
    }
  }

  Future<void> _notifySession(DeviceSessionEntry entry) async {
    try {
      await ref.read(notificationServiceProvider).handleUpdate(
            device: entry.device,
            connectionState: entry.connectionState,
            status: entry.status,
            fault: entry.fault,
          );
    } catch (_) {
      // Notifications optional in tests / when plugin unavailable.
    }
  }

  Future<void> _tearDownSession(String deviceId, DeviceSessionEntry entry) async {
    ref.read(notificationServiceProvider).resetDevice(deviceId);
    final subs = _subscriptions.remove(deviceId);
    if (subs != null) {
      for (final sub in subs) {
        await sub.cancel();
      }
    }
    try {
      await entry.client.disconnect();
    } catch (_) {}
    if (entry.client is FakePhilarmonyWsClient) {
      (entry.client as FakePhilarmonyWsClient).dispose();
    }
  }

  void _disposeAll() {
    final entries = Map<String, DeviceSessionEntry>.from(state.sessions);
    for (final entry in entries.entries) {
      final subs = _subscriptions.remove(entry.key);
      if (subs != null) {
        for (final sub in subs) {
          sub.cancel();
        }
      }
      if (entry.value.client is FakePhilarmonyWsClient) {
        (entry.value.client as FakePhilarmonyWsClient).dispose();
      }
    }
  }
}

final sessionManagerProvider =
    NotifierProvider<SessionManagerNotifier, SessionManagerState>(
  SessionManagerNotifier.new,
);
