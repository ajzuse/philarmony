/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';

/// Holds a finalized cycle map until the shell navigates to [CycleResultPage].
class CycleNavigationNotifier extends Notifier<Map<String, dynamic>?> {
  @override
  Map<String, dynamic>? build() => null;

  void setPending(Map<String, dynamic> cycle) => state = cycle;

  void clear() => state = null;
}

final cycleNavigationProvider =
    NotifierProvider<CycleNavigationNotifier, Map<String, dynamic>?>(
  CycleNavigationNotifier.new,
);
