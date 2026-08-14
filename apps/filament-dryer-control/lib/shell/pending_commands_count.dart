/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';

import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../device/session_deps.dart';
import '../device/session_manager.dart';

/// Queued pending-command count for the active device (shell badge).
final pendingCommandsCountProvider =
    NotifierProvider<PendingCommandsCountNotifier, int>(
  PendingCommandsCountNotifier.new,
);

class PendingCommandsCountNotifier extends Notifier<int> {
  @override
  int build() => 0;

  /// Re-read queue length for [deviceId], or active device when null.
  Future<void> refresh([String? deviceId]) async {
    final id = deviceId ?? ref.read(sessionManagerProvider).activeDeviceId;
    if (id == null) {
      state = 0;
      return;
    }
    try {
      final repo = await ref.read(pendingCommandRepositoryProvider.future);
      state = (await repo.listQueued(id)).length;
    } on StateError {
      // Container disposed (tests).
    }
  }

  /// Schedule refresh after the current provider frame (avoids Riverpod cycles).
  void scheduleRefresh([String? deviceId]) {
    unawaited(Future.microtask(() => refresh(deviceId)));
  }
}
