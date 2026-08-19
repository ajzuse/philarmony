/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../device/device_interfaces.dart';
import '../features/devices/device_settings_page.dart';
import '../shell/app_messenger.dart';
import 'local_notifications.dart';

final notificationServiceProvider = Provider<NotificationService>((ref) {
  return NotificationService(
    local: ref.watch(localNotificationsProvider),
    messenger: ref.watch(appMessengerProvider),
  );
});

/// Fires local notifications and in-app banners on session transitions.
class NotificationService {
  NotificationService({
    required LocalNotifications local,
    required AppMessenger messenger,
  })  : _local = local,
        _messenger = messenger;

  final LocalNotifications _local;
  final AppMessenger _messenger;

  final _prevConnection = <String, DeviceConnectionState>{};
  final _prevStatus = <String, DryerStatus?>{};
  final _prevSensorError = <String, bool>{};
  final _prevFaultCode = <String, String?>{};

  Future<void> handleUpdate({
    required KnownDevice device,
    required DeviceConnectionState connectionState,
    StatusSnapshot? status,
    FaultEvent? fault,
  }) async {
    await _checkConnectionLost(device, connectionState);
    if (status != null) {
      await _checkCycleComplete(device, status);
      await _checkSensorError(device, status);
    }
    if (fault != null) {
      await _checkFault(device, fault);
    }
  }

  Future<void> _checkConnectionLost(
    KnownDevice device,
    DeviceConnectionState connectionState,
  ) async {
    final prev = _prevConnection[device.id];
    _prevConnection[device.id] = connectionState;

    if (prev == null) return;
    final wasOnline = prev == DeviceConnectionState.connected ||
        prev == DeviceConnectionState.reconnecting;
    final isOffline = connectionState == DeviceConnectionState.disconnected ||
        connectionState == DeviceConnectionState.error;
    if (!wasOnline || !isOffline) return;

    await _fire(
      device: device,
      eventKey: DeviceNotificationEvents.connectionLost,
      title: device.nickname,
      body: 'Connection lost',
    );
  }

  Future<void> _checkCycleComplete(
    KnownDevice device,
    StatusSnapshot status,
  ) async {
    final prev = _prevStatus[device.id];
    _prevStatus[device.id] = status.status;

    if (prev != DryerStatus.drying) return;

    if (status.status == DryerStatus.error) {
      await _fire(
        device: device,
        eventKey: DeviceNotificationEvents.safetyFault,
        title: device.nickname,
        body: 'Safety cutoff triggered',
      );
      return;
    }

    if (status.status != DryerStatus.stopped &&
        status.status != DryerStatus.idle) {
      return;
    }

    await _fire(
      device: device,
      eventKey: DeviceNotificationEvents.cycleComplete,
      title: device.nickname,
      body: 'Drying cycle complete',
    );
  }

  Future<void> _checkSensorError(
    KnownDevice device,
    StatusSnapshot status,
  ) async {
    final hadError = _prevSensorError[device.id] ?? false;
    _prevSensorError[device.id] = status.hasSensorError;

    if (hadError || !status.hasSensorError) return;

    await _fire(
      device: device,
      eventKey: DeviceNotificationEvents.cycleError,
      title: device.nickname,
      body: 'Sensor error detected',
    );
  }

  Future<void> _checkFault(KnownDevice device, FaultEvent fault) async {
    final prev = _prevFaultCode[device.id];
    _prevFaultCode[device.id] = fault.faultCode;

    if (prev == fault.faultCode) return;

    await _fire(
      device: device,
      eventKey: DeviceNotificationEvents.safetyFault,
      title: device.nickname,
      body: fault.message.isNotEmpty ? fault.message : fault.faultCode,
    );
  }

  Future<void> _fire({
    required KnownDevice device,
    required String eventKey,
    required String title,
    required String body,
  }) async {
    if (device.notificationSettings[eventKey] == false) return;

    final id = '${device.id}:$eventKey'.hashCode;
    await _local.show(title: title, body: body, id: id);
    _messenger.showSnackBar('$title: $body');
  }

  void resetDevice(String deviceId) {
    _prevConnection.remove(deviceId);
    _prevStatus.remove(deviceId);
    _prevSensorError.remove(deviceId);
    _prevFaultCode.remove(deviceId);
  }
}
