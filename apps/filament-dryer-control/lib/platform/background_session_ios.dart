/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/services.dart';

import '../device/device_interfaces.dart';

/// iOS background networking session hook for active drying cycles.
///
/// Calls `philarmony/background` MethodChannel. Native handler is currently a
/// stub that returns `true` — sustained WS depends on `UIBackgroundModes` in
/// Info.plist (`fetch`, `processing`). OS denial would surface as `false`.
class IosBackgroundSession implements BackgroundSession {
  static const _channel = MethodChannel('philarmony/background');

  bool _active = false;

  @override
  bool get isActive => _active;

  @override
  Future<bool> start({required String reason}) async {
    try {
      final allowed = await _channel.invokeMethod<bool>(
        'start',
        <String, dynamic>{'reason': reason},
      );
      _active = allowed ?? false;
      return _active;
    } on PlatformException {
      _active = false;
      return false;
    } on MissingPluginException {
      // Simulator/test without native channel — allow in-process lock.
      _active = true;
      return true;
    }
  }

  @override
  Future<void> stop() async {
    try {
      await _channel.invokeMethod<void>('stop');
    } on PlatformException {
      // Already stopped.
    } on MissingPluginException {
      // Test host.
    }
    _active = false;
  }
}
