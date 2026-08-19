/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:philarmony_core/philarmony_core.dart';

import 'device_interfaces.dart';
import 'device_runtime_info.dart';

class DeviceSessionState {
  const DeviceSessionState({
    this.activeDevice,
    this.connectionState = DeviceConnectionState.idle,
    this.status,
    this.fault,
    this.backgroundAllowed = true,
    this.activeCycleId,
    this.deviceRuntimeInfo,
    this.hardwareCache,
  });

  final KnownDevice? activeDevice;
  final DeviceConnectionState connectionState;
  final StatusSnapshot? status;
  final FaultEvent? fault;
  final bool backgroundAllowed;
  final String? activeCycleId;
  final DeviceRuntimeInfo? deviceRuntimeInfo;
  final HardwareConfig? hardwareCache;

  DeviceSessionState copyWith({
    KnownDevice? activeDevice,
    DeviceConnectionState? connectionState,
    StatusSnapshot? status,
    FaultEvent? fault,
    bool? backgroundAllowed,
    String? activeCycleId,
    bool clearActiveCycleId = false,
    DeviceRuntimeInfo? deviceRuntimeInfo,
    HardwareConfig? hardwareCache,
  }) {
    return DeviceSessionState(
      activeDevice: activeDevice ?? this.activeDevice,
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
