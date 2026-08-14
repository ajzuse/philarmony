/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:philarmony_core/philarmony_core.dart';

/// Merge rules for offline config edits (FR-010): device wins on conflict.
abstract final class ConfigMergePolicy {
  /// Returns true when the device copy should replace a local draft.
  static bool deviceWins({
    required DateTime? deviceUpdatedAt,
    required DateTime? localEditedAt,
  }) {
    if (deviceUpdatedAt == null) return false;
    if (localEditedAt == null) return true;
    return !deviceUpdatedAt.isBefore(localEditedAt);
  }

  /// Picks the authoritative hardware config when timestamps disagree.
  static HardwareConfig resolveHardwareConflict({
    required HardwareConfig local,
    required HardwareConfig device,
    required DateTime? deviceUpdatedAt,
    required DateTime? localEditedAt,
  }) {
    if (deviceWins(
      deviceUpdatedAt: deviceUpdatedAt,
      localEditedAt: localEditedAt,
    )) {
      return device.copyWith();
    }
    return local.copyWith();
  }

  /// When flushing pending config commands, skip if device already applied
  /// a newer change (device timestamp wins).
  static bool shouldSkipPendingConfig({
    required DateTime pendingCreatedAt,
    required DateTime? deviceUpdatedAt,
  }) {
    if (deviceUpdatedAt == null) return false;
    return deviceUpdatedAt.isAfter(pendingCreatedAt);
  }
}
