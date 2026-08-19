/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:intl/intl.dart';

/// Multi-cycle trend chart showing average temperature per cycle over time.
class HistoryTrendsChart extends StatelessWidget {
  const HistoryTrendsChart({super.key, required this.cycles});

  final List<Map<String, dynamic>> cycles;

  @override
  Widget build(BuildContext context) {
    final points = <_TrendPoint>[];
    for (final c in cycles) {
      final avg = c['avg_temp_c'];
      final started = c['started_at'] as String?;
      if (avg is! num || started == null) continue;
      final date = DateTime.tryParse(started);
      if (date == null) continue;
      points.add(_TrendPoint(date: date, avgTemp: avg.toDouble()));
    }

    if (points.length < 2) {
      return const SizedBox.shrink();
    }

    points.sort((a, b) => a.date.compareTo(b.date));
    final spots = <FlSpot>[];
    for (var i = 0; i < points.length; i++) {
      spots.add(FlSpot(i.toDouble(), points[i].avgTemp));
    }

    final minY = points.map((p) => p.avgTemp).reduce((a, b) => a < b ? a : b);
    final maxY = points.map((p) => p.avgTemp).reduce((a, b) => a > b ? a : b);
    final dateFmt = DateFormat.MMMd();

    return Card(
      margin: const EdgeInsets.fromLTRB(16, 8, 16, 0),
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Text(
              'Avg temp trend',
              style: Theme.of(context).textTheme.titleSmall,
            ),
            const SizedBox(height: 8),
            SizedBox(
              height: 120,
              child: LineChart(
                LineChartData(
                  minY: (minY - 5).clamp(0, double.infinity),
                  maxY: maxY + 5,
                  lineBarsData: [
                    LineChartBarData(
                      spots: spots,
                      isCurved: true,
                      color: Theme.of(context).colorScheme.primary,
                      dotData: FlDotData(
                        show: points.length <= 12,
                      ),
                      barWidth: 2,
                    ),
                  ],
                  titlesData: FlTitlesData(
                    topTitles: const AxisTitles(
                      sideTitles: SideTitles(showTitles: false),
                    ),
                    rightTitles: const AxisTitles(
                      sideTitles: SideTitles(showTitles: false),
                    ),
                    leftTitles: AxisTitles(
                      sideTitles: SideTitles(
                        showTitles: true,
                        reservedSize: 36,
                        getTitlesWidget: (value, meta) => Text(
                          value.toStringAsFixed(0),
                          style: Theme.of(context).textTheme.labelSmall,
                        ),
                      ),
                    ),
                    bottomTitles: AxisTitles(
                      sideTitles: SideTitles(
                        showTitles: true,
                        reservedSize: 22,
                        interval: (points.length / 4).ceilToDouble().clamp(1, 999),
                        getTitlesWidget: (value, meta) {
                          final i = value.round();
                          if (i < 0 || i >= points.length) {
                            return const SizedBox.shrink();
                          }
                          return Text(
                            dateFmt.format(points[i].date),
                            style: Theme.of(context).textTheme.labelSmall,
                          );
                        },
                      ),
                    ),
                  ),
                  gridData: const FlGridData(show: true, drawVerticalLine: false),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class _TrendPoint {
  const _TrendPoint({required this.date, required this.avgTemp});

  final DateTime date;
  final double avgTemp;
}
