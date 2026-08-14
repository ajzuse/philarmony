/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:intl/intl.dart';

import '../../data/app_preferences.dart';
import '../../device/session_deps.dart';
import '../../l10n/app_localizations.dart';
import '../settings/unit_format.dart';
import 'history_aggregate_stats.dart';
import 'history_controller.dart';
import 'history_trends.dart';

class HistoryPage extends ConsumerWidget {
  const HistoryPage({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final filters = ref.watch(historyFiltersProvider);
    final cyclesAsync = ref.watch(filteredCyclesProvider);
    final selectedId = ref.watch(selectedCycleIdProvider);
    final materialsAsync = ref.watch(availableMaterialsProvider);
    final devicesAsync = ref.watch(knownDeviceRepositoryProvider);
    final prefs = ref.watch(appPreferencesProvider).valueOrNull;
    final useCelsius = prefs?.useCelsius ?? true;
    final timeFormat = prefs?.timeFormat ?? AppTimeFormat.h24;
    final dateFmt = DateFormat.yMMMd(Localizations.localeOf(context).toString());

    return Column(
      children: [
        SingleChildScrollView(
          scrollDirection: Axis.horizontal,
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
          child: Row(
            children: [
              _FilterChip(
                label: 'All',
                selected: filters.status == 'all',
                onTap: () => _setStatus(ref, 'all'),
              ),
              _FilterChip(
                label: 'Completed',
                selected: filters.status == 'completed',
                onTap: () => _setStatus(ref, 'completed'),
              ),
              _FilterChip(
                label: 'In progress',
                selected: filters.status == 'in_progress',
                onTap: () => _setStatus(ref, 'in_progress'),
              ),
              const SizedBox(width: 8),
              _SortMenu(
                sort: filters.sort,
                onChanged: (sort) => ref
                    .read(historyFiltersProvider.notifier)
                    .update((f) => f.copyWith(sort: sort)),
              ),
            ],
          ),
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 16),
          child: Row(
            children: [
              Expanded(
                child: OutlinedButton.icon(
                  onPressed: () => _pickDateRange(context, ref, filters),
                  icon: const Icon(Icons.date_range, size: 18),
                  label: Text(
                    filters.dateRange == null
                        ? 'Date range'
                        : '${dateFmt.format(filters.dateRange!.start)} – '
                            '${dateFmt.format(filters.dateRange!.end)}',
                    overflow: TextOverflow.ellipsis,
                  ),
                ),
              ),
              if (filters.dateRange != null)
                IconButton(
                  tooltip: 'Clear date',
                  onPressed: () => ref
                      .read(historyFiltersProvider.notifier)
                      .update((f) => f.copyWith(clearDateRange: true)),
                  icon: const Icon(Icons.clear, size: 18),
                ),
            ],
          ),
        ),
        materialsAsync.when(
          data: (materials) {
            if (materials.isEmpty) return const SizedBox.shrink();
            return SingleChildScrollView(
              scrollDirection: Axis.horizontal,
              padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
              child: Row(
                children: [
                  _FilterChip(
                    label: 'All materials',
                    selected: filters.material == null,
                    onTap: () => ref
                        .read(historyFiltersProvider.notifier)
                        .update((f) => f.copyWith(clearMaterial: true)),
                  ),
                  for (final m in materials)
                    _FilterChip(
                      label: m,
                      selected: filters.material == m,
                      onTap: () => ref
                          .read(historyFiltersProvider.notifier)
                          .update((f) => f.copyWith(material: m)),
                    ),
                ],
              ),
            );
          },
          loading: () => const SizedBox.shrink(),
          error: (_, __) => const SizedBox.shrink(),
        ),
        devicesAsync.when(
          data: (repo) {
            return FutureBuilder(
              future: repo.list(),
              builder: (context, snapshot) {
                final devices = snapshot.data ?? [];
                if (devices.isEmpty) return const SizedBox.shrink();
                return SingleChildScrollView(
                  scrollDirection: Axis.horizontal,
                  padding:
                      const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
                  child: Row(
                    children: [
                      _FilterChip(
                        label: 'All devices',
                        selected: filters.deviceId == null,
                        onTap: () => ref
                            .read(historyFiltersProvider.notifier)
                            .update((f) => f.copyWith(clearDevice: true)),
                      ),
                      for (final d in devices)
                        _FilterChip(
                          label: d.nickname.isNotEmpty ? d.nickname : d.host,
                          selected: filters.deviceId == d.id,
                          onTap: () => ref
                              .read(historyFiltersProvider.notifier)
                              .update((f) => f.copyWith(deviceId: d.id)),
                        ),
                    ],
                  ),
                );
              },
            );
          },
          loading: () => const SizedBox.shrink(),
          error: (_, __) => const SizedBox.shrink(),
        ),
        cyclesAsync.when(
          data: (cycles) {
            final stats = computeHistoryStats(cycles);
            return Column(
              children: [
                HistoryAggregateStatsBar(stats: stats),
                HistoryTrendsChart(cycles: cycles),
              ],
            );
          },
          loading: () => const SizedBox.shrink(),
          error: (_, __) => const SizedBox.shrink(),
        ),
        Expanded(
          child: cyclesAsync.when(
            data: (cycles) {
              if (cycles.isEmpty) {
                return Center(child: Text('${l10n.navHistory} — empty'));
              }
              return ListView.builder(
                itemCount: cycles.length,
                itemBuilder: (context, i) {
                  final c = cycles[i];
                  final id = c['id']?.toString();
                  final started = c['started_at'] as String?;
                  final dateLabel = started != null
                      ? dateFmt.format(DateTime.parse(started).toLocal())
                      : '—';
                  final targetTemp = formatTemperature(
                    (c['target_temp_c'] as num?)?.toDouble(),
                    useCelsius: useCelsius,
                  );
                  final avgTemp = formatTemperature(
                    (c['avg_temp_c'] as num?)?.toDouble(),
                    useCelsius: useCelsius,
                  );
                  final duration = cycleDurationSec(c);
                  final durationLabel = duration != null
                      ? formatDuration(duration, timeFormat: timeFormat)
                      : '—';

                  return ListTile(
                    selected: id != null && id == selectedId,
                    title: Text(c['material_name']?.toString() ?? 'Cycle'),
                    subtitle: Text(
                      '$dateLabel · $targetTemp target · $avgTemp avg · '
                      '$durationLabel',
                    ),
                    isThreeLine: true,
                    onTap: () {
                      if (id != null) {
                        ref.read(selectedCycleIdProvider.notifier).state = id;
                      }
                    },
                  );
                },
              );
            },
            loading: () => const Center(child: CircularProgressIndicator()),
            error: (e, _) => Center(child: Text('$e')),
          ),
        ),
      ],
    );
  }

  void _setStatus(WidgetRef ref, String status) {
    ref
        .read(historyFiltersProvider.notifier)
        .update((f) => f.copyWith(status: status));
  }

  Future<void> _pickDateRange(
    BuildContext context,
    WidgetRef ref,
    HistoryFilters filters,
  ) async {
    final range = await showDateRangePicker(
      context: context,
      firstDate: DateTime(2020),
      lastDate: DateTime.now().add(const Duration(days: 1)),
      initialDateRange: filters.dateRange,
    );
    if (range != null) {
      ref
          .read(historyFiltersProvider.notifier)
          .update((f) => f.copyWith(dateRange: range));
    }
  }
}

class _FilterChip extends StatelessWidget {
  const _FilterChip({
    required this.label,
    required this.selected,
    required this.onTap,
  });

  final String label;
  final bool selected;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(right: 8),
      child: FilterChip(
        label: Text(label),
        selected: selected,
        onSelected: (_) => onTap(),
      ),
    );
  }
}

class _SortMenu extends StatelessWidget {
  const _SortMenu({required this.sort, required this.onChanged});

  final HistorySort sort;
  final ValueChanged<HistorySort> onChanged;

  @override
  Widget build(BuildContext context) {
    return PopupMenuButton<HistorySort>(
      tooltip: 'Sort',
      icon: const Icon(Icons.sort),
      initialValue: sort,
      onSelected: onChanged,
      itemBuilder: (context) => const [
        PopupMenuItem(value: HistorySort.dateDesc, child: Text('Newest first')),
        PopupMenuItem(value: HistorySort.dateAsc, child: Text('Oldest first')),
        PopupMenuItem(
          value: HistorySort.durationDesc,
          child: Text('Longest duration'),
        ),
        PopupMenuItem(
          value: HistorySort.tempDesc,
          child: Text('Highest avg temp'),
        ),
      ],
    );
  }
}
