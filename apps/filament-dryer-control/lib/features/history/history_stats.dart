/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';

class CycleStatsRow extends StatelessWidget {
  const CycleStatsRow({super.key, required this.cycle});

  final Map<String, dynamic> cycle;

  @override
  Widget build(BuildContext context) {
    return Wrap(
      spacing: 12,
      runSpacing: 8,
      children: [
        _StatChip(
          label: 'Avg temp',
          value: _fmt(cycle['avg_temp_c'], suffix: ' °C'),
        ),
        _StatChip(
          label: 'Max temp',
          value: _fmt(cycle['max_temp_c'], suffix: ' °C'),
        ),
        _StatChip(
          label: 'Avg humidity',
          value: _fmt(cycle['avg_humidity_pct'], suffix: ' %'),
        ),
        _StatChip(
          label: 'Samples',
          value: '${_sampleCount(cycle)}',
        ),
      ],
    );
  }

  static int _sampleCount(Map<String, dynamic> cycle) {
    final samples = cycle['samples'];
    if (samples is List) return samples.length;
    return 0;
  }

  static String _fmt(dynamic v, {String suffix = ''}) {
    if (v == null) return '—';
    if (v is num) return '${v.toStringAsFixed(1)}$suffix';
    return '$v$suffix';
  }
}

class _StatChip extends StatelessWidget {
  const _StatChip({required this.label, required this.value});

  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    return Chip(
      label: Text('$label: $value'),
      visualDensity: VisualDensity.compact,
    );
  }
}
