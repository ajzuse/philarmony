/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';

import 'history_controller.dart';

class HistoryAggregateStats {
  const HistoryAggregateStats({
    required this.totalCycles,
    required this.totalHours,
    required this.avgDurationSec,
    required this.successRate,
  });

  final int totalCycles;
  final double totalHours;
  final int avgDurationSec;
  final double successRate;
}

HistoryAggregateStats computeHistoryStats(List<Map<String, dynamic>> cycles) {
  if (cycles.isEmpty) {
    return const HistoryAggregateStats(
      totalCycles: 0,
      totalHours: 0,
      avgDurationSec: 0,
      successRate: 0,
    );
  }

  var totalSec = 0;
  var durationCount = 0;
  var completedCount = 0;
  var successCount = 0;

  for (final c in cycles) {
    final duration = cycleDurationSec(c);
    if (duration != null && duration > 0) {
      totalSec += duration;
      durationCount++;
    }
    if (c['status'] == 'completed') {
      completedCount++;
      final reason = c['stop_reason']?.toString() ?? '';
      if (reason == 'completed') successCount++;
    }
  }

  final avgDuration =
      durationCount > 0 ? (totalSec / durationCount).round() : 0;
  final successRate =
      completedCount > 0 ? successCount / completedCount : 0.0;

  return HistoryAggregateStats(
    totalCycles: cycles.length,
    totalHours: totalSec / 3600,
    avgDurationSec: avgDuration,
    successRate: successRate,
  );
}

class HistoryAggregateStatsBar extends StatelessWidget {
  const HistoryAggregateStatsBar({super.key, required this.stats});

  final HistoryAggregateStats stats;

  @override
  Widget build(BuildContext context) {
    if (stats.totalCycles == 0) return const SizedBox.shrink();

    return Card(
      margin: const EdgeInsets.fromLTRB(16, 8, 16, 0),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 10),
        child: Wrap(
          spacing: 16,
          runSpacing: 8,
          children: [
            _Stat(label: 'Cycles', value: '${stats.totalCycles}'),
            _Stat(
              label: 'Total hours',
              value: stats.totalHours.toStringAsFixed(1),
            ),
            _Stat(
              label: 'Avg duration',
              value: _formatDuration(stats.avgDurationSec),
            ),
            _Stat(
              label: 'Success rate',
              value: '${(stats.successRate * 100).toStringAsFixed(0)}%',
            ),
          ],
        ),
      ),
    );
  }

  static String _formatDuration(int totalSeconds) {
    final hours = totalSeconds ~/ 3600;
    final minutes = (totalSeconds % 3600) ~/ 60;
    if (hours > 0) return '${hours}h ${minutes}m';
    if (minutes > 0) return '${minutes}m';
    return '${totalSeconds}s';
  }
}

class _Stat extends StatelessWidget {
  const _Stat({required this.label, required this.value});

  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        Text(
          label,
          style: Theme.of(context).textTheme.labelSmall,
        ),
        Text(
          value,
          style: Theme.of(context).textTheme.titleSmall,
        ),
      ],
    );
  }
}
