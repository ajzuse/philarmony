/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';

class LiveCharts extends StatelessWidget {
  const LiveCharts({
    super.key,
    required this.temps,
    required this.humidity,
    required this.heaterPower,
    this.chartWindowMinutes = 60,
  });

  final List<double> temps;
  final List<double> humidity;
  final List<double> heaterPower;
  final int chartWindowMinutes;

  @override
  Widget build(BuildContext context) {
    final hasData =
        temps.isNotEmpty || humidity.isNotEmpty || heaterPower.isNotEmpty;
    if (!hasData) {
      return const Center(child: Text('Charts — waiting for data'));
    }

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text(
          'Last $chartWindowMinutes min',
          style: Theme.of(context).textTheme.labelMedium,
        ),
        const SizedBox(height: 8),
        _SeriesChart(title: 'Temperature', data: temps, color: Colors.orange),
        const SizedBox(height: 12),
        _SeriesChart(title: 'Humidity', data: humidity, color: Colors.blue),
        const SizedBox(height: 12),
        _SeriesChart(
          title: 'Heater power',
          data: heaterPower,
          color: Colors.red,
          unit: '%',
        ),
      ],
    );
  }
}

class _SeriesChart extends StatelessWidget {
  const _SeriesChart({
    required this.title,
    required this.data,
    required this.color,
    this.unit,
  });

  final String title;
  final List<double> data;
  final Color color;
  final String? unit;

  @override
  Widget build(BuildContext context) {
    final latest = data.isEmpty ? '—' : data.last.toStringAsFixed(1);
    final suffix = unit == null ? '' : ' $unit';
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Text(title, style: Theme.of(context).textTheme.titleSmall),
                const Spacer(),
                Text('$latest$suffix'),
              ],
            ),
            const SizedBox(height: 8),
            SizedBox(
              height: 100,
              child: data.isEmpty
                  ? const Center(child: Text('—'))
                  : LineChart(
                      LineChartData(
                        lineBarsData: [
                          LineChartBarData(
                            spots: [
                              for (var i = 0; i < data.length; i++)
                                FlSpot(i.toDouble(), data[i]),
                            ],
                            isCurved: true,
                            color: color,
                            dotData: const FlDotData(show: false),
                          ),
                        ],
                        titlesData: const FlTitlesData(show: false),
                        gridData: const FlGridData(show: false),
                        borderData: FlBorderData(show: false),
                      ),
                    ),
            ),
          ],
        ),
      ),
    );
  }
}
