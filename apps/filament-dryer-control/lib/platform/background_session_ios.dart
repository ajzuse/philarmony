/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import '../device/device_interfaces.dart';

/// iOS background networking session hook for active drying cycles.
///
/// Relies on `UIBackgroundModes` = `fetch` / `processing` in Info.plist and
/// Local Network usage description. Keeps an in-process active flag for tests.
class IosBackgroundSession implements BackgroundSession {
  bool _active = false;

  @override
  bool get isActive => _active;

  @override
  Future<bool> start({required String reason}) async {
    _active = true;
    return true;
  }

  @override
  Future<void> stop() async {
    _active = false;
  }
}
