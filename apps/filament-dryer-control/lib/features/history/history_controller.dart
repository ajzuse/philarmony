/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../device/session_deps.dart';

enum HistorySort { dateDesc, dateAsc, durationDesc, tempDesc }

class HistoryFilters {
  const HistoryFilters({
    this.status = 'all',
    this.dateRange,
    this.material,
    this.deviceId,
    this.sort = HistorySort.dateDesc,
  });

  final String status;
  final DateTimeRange? dateRange;
  final String? material;
  final String? deviceId;
  final HistorySort sort;

  HistoryFilters copyWith({
    String? status,
    DateTimeRange? dateRange,
    bool clearDateRange = false,
    String? material,
    bool clearMaterial = false,
    String? deviceId,
    bool clearDevice = false,
    HistorySort? sort,
  }) {
    return HistoryFilters(
      status: status ?? this.status,
      dateRange: clearDateRange ? null : (dateRange ?? this.dateRange),
      material: clearMaterial ? null : (material ?? this.material),
      deviceId: clearDevice ? null : (deviceId ?? this.deviceId),
      sort: sort ?? this.sort,
    );
  }
}

final historyCyclesProvider = FutureProvider<List<Map<String, dynamic>>>((ref) async {
  final repo = await ref.watch(dryingCycleRepositoryProvider.future);
  return repo.list();
});

final historyFiltersProvider =
    StateProvider<HistoryFilters>((ref) => const HistoryFilters());

/// Legacy alias for status-only filter chips.
final historyFilterProvider = Provider<String>((ref) {
  return ref.watch(historyFiltersProvider).status;
});

final availableMaterialsProvider = Provider<AsyncValue<List<String>>>((ref) {
  return ref.watch(historyCyclesProvider).whenData((cycles) {
    final names = <String>{};
    for (final c in cycles) {
      final name = c['material_name']?.toString();
      if (name != null && name.isNotEmpty) names.add(name);
    }
    final list = names.toList()..sort();
    return list;
  });
});

final filteredCyclesProvider = Provider<AsyncValue<List<Map<String, dynamic>>>>((ref) {
  final filters = ref.watch(historyFiltersProvider);
  return ref.watch(historyCyclesProvider).whenData((cycles) {
    var result = cycles.where((c) {
      if (filters.status != 'all' && c['status'] != filters.status) {
        return false;
      }
      if (filters.material != null &&
          c['material_name']?.toString() != filters.material) {
        return false;
      }
      if (filters.deviceId != null &&
          c['device_id']?.toString() != filters.deviceId) {
        return false;
      }
      if (filters.dateRange != null) {
        final started = _parseStartedAt(c);
        if (started == null) return false;
        final range = filters.dateRange!;
        if (started.isBefore(range.start) ||
            started.isAfter(range.end.add(const Duration(days: 1)))) {
          return false;
        }
      }
      return true;
    }).toList();

    result = List<Map<String, dynamic>>.from(result);
    switch (filters.sort) {
      case HistorySort.dateDesc:
        result.sort(
          (a, b) => (_parseStartedAt(b) ?? DateTime(0))
              .compareTo(_parseStartedAt(a) ?? DateTime(0)),
        );
      case HistorySort.dateAsc:
        result.sort(
          (a, b) => (_parseStartedAt(a) ?? DateTime(0))
              .compareTo(_parseStartedAt(b) ?? DateTime(0)),
        );
      case HistorySort.durationDesc:
        result.sort(
          (a, b) => (cycleDurationSec(b) ?? 0).compareTo(cycleDurationSec(a) ?? 0),
        );
      case HistorySort.tempDesc:
        result.sort((a, b) {
          final ta = (a['avg_temp_c'] as num?)?.toDouble() ?? 0;
          final tb = (b['avg_temp_c'] as num?)?.toDouble() ?? 0;
          return tb.compareTo(ta);
        });
    }
    return result;
  });
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

DateTime? _parseStartedAt(Map<String, dynamic> cycle) {
  final raw = cycle['started_at'] as String?;
  if (raw == null) return null;
  return DateTime.tryParse(raw);
}

/// Returns cycle duration in seconds from timestamps or last sample.
int? cycleDurationSec(Map<String, dynamic> cycle) {
  final started = cycle['started_at'] as String?;
  final ended = cycle['ended_at'] as String?;
  if (started != null && ended != null) {
    final start = DateTime.tryParse(started);
    final end = DateTime.tryParse(ended);
    if (start != null && end != null) {
      return end.difference(start).inSeconds;
    }
  }
  final samples = cycle['samples'];
  if (samples is List && samples.isNotEmpty) {
    final last = samples.last;
    if (last is Map) {
      final t = last['t_sec'];
      if (t is num) return t.toInt();
    }
  }
  return null;
}
