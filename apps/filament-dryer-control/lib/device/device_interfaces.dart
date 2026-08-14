/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';

import 'package:philarmony_core/philarmony_core.dart';

import 'device_runtime_info.dart';

enum DeviceConnectionState {
  idle,
  connecting,
  connected,
  reconnecting,
  disconnected,
  error,
}

class DiscoveredDevice {
  const DiscoveredDevice({
    required this.name,
    required this.host,
    this.port = 80,
  });

  final String name;
  final String host;
  final int port;
}

abstract class DeviceDiscovery {
  Stream<List<DiscoveredDevice>> scan({Duration timeout = const Duration(seconds: 5)});
}

abstract class PhilarmonyWsClient {
  Stream<DeviceConnectionState> get connectionStates;
  Stream<StatusSnapshot> get statusStream;
  Stream<FaultEvent> get faultStream;
  Stream<HardwareConfigResponse> get hardwareConfigResponseStream;
  Stream<HardwareConfigError> get hardwareConfigErrorStream;

  DeviceConnectionState get state;
  StatusSnapshot? get lastStatus;
  HardwareConfig? get lastHardwareConfig;
  DeviceRuntimeInfo? get deviceRuntimeInfo;
  DateTime? get hardwareConfigUpdatedAt;

  Future<void> connect(KnownDevice device);
  Future<void> disconnect();
  Future<void> send(WsEnvelope envelope);
  Future<WsEnvelope?> request(
    WsEnvelope envelope, {
    Duration timeout = const Duration(seconds: 5),
  });
  Future<void> subscribeStatus();
  Future<void> startCycle(StartCycleRequest request);
  Future<void> stopCycle([StopCycleRequest request]);
  Future<void> sendHardwareConfig(HardwareConfig config);
}

abstract class BackgroundSession {
  Future<bool> start({required String reason});
  Future<void> stop();
  bool get isActive;
}
