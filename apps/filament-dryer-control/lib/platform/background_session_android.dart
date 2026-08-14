/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import '../device/device_interfaces.dart';

/// Android foreground-service style background session for active drying cycles.
///
/// Declares FGS intent via platform channel when available; falls back to an
/// in-process active flag so session lifecycle still works in tests/desktop.
class AndroidBackgroundSession implements BackgroundSession {
  bool _active = false;

  @override
  bool get isActive => _active;

  @override
  Future<bool> start({required String reason}) async {
    // Manifest declares FOREGROUND_SERVICE + dataSync type; native plugin wiring
    // can promote this to a real FGS. Until then, keep process-local lock.
    _active = true;
    return true;
  }

  @override
  Future<void> stop() async {
    _active = false;
  }
}
