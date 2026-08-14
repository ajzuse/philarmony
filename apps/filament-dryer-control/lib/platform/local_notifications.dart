/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

final localNotificationsProvider = Provider<LocalNotifications>((ref) {
  return LocalNotifications();
});

/// Platform local notification adapter (mobile + desktop via plugin).
class LocalNotifications {
  final FlutterLocalNotificationsPlugin _plugin =
      FlutterLocalNotificationsPlugin();

  bool _initialized = false;

  Future<void> init() async {
    if (_initialized) return;

    const android = AndroidInitializationSettings('@mipmap/ic_launcher');
    const ios = DarwinInitializationSettings();
    const linux = LinuxInitializationSettings(defaultActionName: 'Open');
    const initSettings = InitializationSettings(
      android: android,
      iOS: ios,
      linux: linux,
    );

    await _plugin.initialize(initSettings);
    _initialized = true;
  }

  Future<void> show({
    required String title,
    required String body,
    int id = 0,
  }) async {
    if (!_initialized) await init();

    const androidDetails = AndroidNotificationDetails(
      'philarmony_alerts',
      'Philarmony alerts',
      channelDescription: 'Cycle and connection alerts',
      importance: Importance.high,
      priority: Priority.high,
    );
    const iosDetails = DarwinNotificationDetails();
    const linuxDetails = LinuxNotificationDetails();
    const details = NotificationDetails(
      android: androidDetails,
      iOS: iosDetails,
      linux: linuxDetails,
    );

    await _plugin.show(id, title, body, details);
  }
}
