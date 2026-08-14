/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../device/session_providers.dart';

final historyCyclesProvider = FutureProvider<List<Map<String, dynamic>>>((ref) async {
  final repo = await ref.watch(dryingCycleRepositoryProvider.future);
  return repo.list();
});

final selectedCycleIdProvider = StateProvider<String?>((ref) => null);

final selectedCycleProvider = Provider<Map<String, dynamic>?>((ref) {
  final id = ref.watch(selectedCycleIdProvider);
  if (id == null) return null;
  final cycles = ref.watch(historyCyclesProvider).valueOrNull ?? [];
  for (final c in cycles) {
    if (c['id'] == id) return c;
  }
  return null;
});

final historyFilterProvider = StateProvider<String>((ref) => 'all');

final filteredCyclesProvider = Provider<AsyncValue<List<Map<String, dynamic>>>>((ref) {
  final filter = ref.watch(historyFilterProvider);
  return ref.watch(historyCyclesProvider).whenData((cycles) {
    if (filter == 'all') return cycles;
    return cycles.where((c) => c['status'] == filter).toList();
  });
});
