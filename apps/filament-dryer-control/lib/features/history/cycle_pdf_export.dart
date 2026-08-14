/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:typed_data';

import 'package:pdf/pdf.dart';
import 'package:pdf/widgets.dart' as pw;

Future<Uint8List> exportCyclePdf(Map<String, dynamic> cycle) async {
  final samples = (cycle['samples'] as List?)?.cast<Map<String, dynamic>>() ?? [];
  final temps = <double>[];
  for (final s in samples) {
    final t = s['chamber_temp_c'];
    if (t is num) temps.add(t.toDouble());
  }

  final doc = pw.Document();
  doc.addPage(
    pw.MultiPage(
      pageFormat: PdfPageFormat.a4,
      build: (ctx) => [
        pw.Text('Philarmony Drying Cycle', style: pw.TextStyle(fontSize: 20)),
        pw.SizedBox(height: 12),
        pw.Text('Material: ${cycle['material_name'] ?? '—'}'),
        pw.Text('Target temp: ${cycle['target_temp_c'] ?? '—'} °C'),
        pw.Text('Stop reason: ${cycle['stop_reason'] ?? '—'}'),
        pw.Text('Avg temp: ${cycle['avg_temp_c'] ?? '—'} °C'),
        pw.Text('Max temp: ${cycle['max_temp_c'] ?? '—'} °C'),
        pw.SizedBox(height: 16),
        if (temps.isNotEmpty) ...[
          pw.Text('Temperature trend', style: pw.TextStyle(fontSize: 14)),
          pw.SizedBox(height: 8),
          pw.Text(
            'Min: ${temps.reduce((a, b) => a < b ? a : b).toStringAsFixed(1)} °C · '
            'Max: ${temps.reduce((a, b) => a > b ? a : b).toStringAsFixed(1)} °C · '
            'Points: ${temps.length}',
          ),
        ] else
          pw.Text('No sample chart data'),
      ],
    ),
  );
  return doc.save();
}
