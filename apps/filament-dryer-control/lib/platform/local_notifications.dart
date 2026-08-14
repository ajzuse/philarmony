/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';

final localNotificationsProvider = Provider<LocalNotifications>((ref) {
  return LocalNotifications();
});

class LocalNotifications {
  bool _initialized = false;

  Future<void> init() async {
    _initialized = true;
  }

  Future<void> show({required String title, required String body}) async {
    if (!_initialized) await init();
    // Platform channel wiring deferred; stub records intent for tests.
  }
}
