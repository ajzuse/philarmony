/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../device/session_providers.dart';

/// Connects to the last-used or auto-connect flagged device on app launch.
final discoverBootstrapProvider = FutureProvider<void>((ref) async {
  final repo = await ref.watch(knownDeviceRepositoryProvider.future);
  final candidate = repo.autoConnectCandidate();
  if (candidate == null) return;
  await ref.read(deviceSessionProvider.notifier).connect(candidate);
});
