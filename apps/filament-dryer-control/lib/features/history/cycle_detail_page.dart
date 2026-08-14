/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:printing/printing.dart';

import '../../l10n/app_localizations.dart';
import '../../platform/cycle_file_export.dart';
import 'cycle_pdf_export.dart';
import 'history_stats.dart';

class CycleDetailPage extends ConsumerWidget {
  const CycleDetailPage({super.key, required this.cycle});

  final Map<String, dynamic> cycle;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final samples = (cycle['samples'] as List?)?.cast<Map<String, dynamic>>() ?? [];

    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Text(
          cycle['material_name']?.toString() ?? 'Cycle',
          style: Theme.of(context).textTheme.titleLarge,
        ),
        const SizedBox(height: 8),
        Text('${cycle['target_temp_c'] ?? '—'} °C target'),
        Text('Stop: ${cycle['stop_reason'] ?? '—'}'),
        const SizedBox(height: 12),
        CycleStatsRow(cycle: cycle),
        const SizedBox(height: 16),
        if (samples.isNotEmpty)
          SizedBox(
            height: 180,
            child: _SampleChart(samples: samples),
          )
        else
          const Text('No sample data'),
        const SizedBox(height: 16),
        FilledButton.icon(
          onPressed: () async {
            final path = await saveCycleCsv(cycle);
            if (context.mounted) {
              ScaffoldMessenger.of(context).showSnackBar(
                SnackBar(content: Text('CSV: $path')),
              );
            }
          },
          icon: const Icon(Icons.table_chart_outlined),
          label: Text(l10n.exportCsv),
        ),
        const SizedBox(height: 8),
        FilledButton.icon(
          onPressed: () async {
            final locale = Localizations.localeOf(context);
            final bytes = await exportCyclePdf(cycle, locale: locale);
            await Printing.sharePdf(bytes: bytes, filename: 'cycle.pdf');
          },
          icon: const Icon(Icons.picture_as_pdf_outlined),
          label: Text(l10n.exportPdf),
        ),
      ],
    );
  }
}

class HistoryDetailPane extends ConsumerWidget {
  const HistoryDetailPane({super.key, required this.cycle});

  final Map<String, dynamic>? cycle;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    if (cycle == null) {
      return const Center(child: Text('Select a cycle'));
    }
    return CycleDetailPage(cycle: cycle!);
  }
}

class _SampleChart extends StatelessWidget {
  const _SampleChart({required this.samples});

  final List<Map<String, dynamic>> samples;

  @override
  Widget build(BuildContext context) {
    final temps = <FlSpot>[];
    for (var i = 0; i < samples.length; i++) {
      final t = samples[i]['chamber_temp_c'];
      if (t is num) temps.add(FlSpot(i.toDouble(), t.toDouble()));
    }
    if (temps.isEmpty) return const SizedBox.shrink();

    return LineChart(
      LineChartData(
        lineBarsData: [
          LineChartBarData(
            spots: temps,
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
