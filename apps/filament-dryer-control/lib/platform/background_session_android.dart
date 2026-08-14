/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/services.dart';

import '../device/device_interfaces.dart';

/// Android foreground-service style background session for active drying cycles.
///
/// Starts [CycleForegroundService] via `philarmony/background` MethodChannel.
/// Returns `false` when the OS denies foreground service start.
class AndroidBackgroundSession implements BackgroundSession {
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
      _active = false;
      return false;
    }
  }

  @override
  Future<void> stop() async {
    try {
      await _channel.invokeMethod<void>('stop');
    } on PlatformException {
      // Service may already be stopped.
    } on MissingPluginException {
      // Desktop/test host without native channel.
    }
    _active = false;
  }
}
