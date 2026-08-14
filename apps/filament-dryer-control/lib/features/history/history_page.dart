/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../l10n/app_localizations.dart';
import 'history_controller.dart';

class HistoryPage extends ConsumerWidget {
  const HistoryPage({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final filter = ref.watch(historyFilterProvider);
    final cyclesAsync = ref.watch(filteredCyclesProvider);
    final selectedId = ref.watch(selectedCycleIdProvider);

    return Column(
      children: [
        SingleChildScrollView(
          scrollDirection: Axis.horizontal,
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
          child: Row(
            children: [
              _FilterChip(
                label: 'All',
                selected: filter == 'all',
                onTap: () => ref.read(historyFilterProvider.notifier).state = 'all',
              ),
              _FilterChip(
                label: 'Completed',
                selected: filter == 'completed',
                onTap: () =>
                    ref.read(historyFilterProvider.notifier).state = 'completed',
              ),
              _FilterChip(
                label: 'In progress',
                selected: filter == 'in_progress',
                onTap: () =>
                    ref.read(historyFilterProvider.notifier).state = 'in_progress',
              ),
            ],
          ),
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
                  return ListTile(
                    selected: id != null && id == selectedId,
                    title: Text(c['material_name']?.toString() ?? 'Cycle'),
                    subtitle: Text(
                      '${c['status'] ?? ''} · ${c['stop_reason']?.toString() ?? '—'}',
                    ),
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
