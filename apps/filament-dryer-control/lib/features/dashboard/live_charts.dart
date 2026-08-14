/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';

class LiveCharts extends StatelessWidget {
  const LiveCharts({super.key, required this.temps});

  final List<double> temps;

  @override
  Widget build(BuildContext context) {
    if (temps.isEmpty) {
      return const Center(child: Text('Charts — waiting for data'));
    }
    return LineChart(
      LineChartData(
        lineBarsData: [
          LineChartBarData(
            spots: [
              for (var i = 0; i < temps.length; i++)
                FlSpot(i.toDouble(), temps[i]),
            ],
            isCurved: true,
            color: Theme.of(context).colorScheme.primary,
            dotData: const FlDotData(show: false),
          ),
        ],
        titlesData: const FlTitlesData(show: false),
        gridData: const FlGridData(show: false),
      ),
    );
  }
}
