/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:math' as math;
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:pdf/pdf.dart';
import 'package:pdf/widgets.dart' as pw;

class _PdfStrings {
  const _PdfStrings({required this.isPt});

  factory _PdfStrings.fromLocale(Locale? locale) {
    final isPt = locale?.languageCode == 'pt';
    return _PdfStrings(isPt: isPt);
  }

  final bool isPt;

  String get title =>
      isPt ? 'Ciclo de secagem Philarmony' : 'Philarmony Drying Cycle';
  String get material => isPt ? 'Material' : 'Material';
  String get targetTemp => isPt ? 'Temp. alvo' : 'Target temp';
  String get stopReason => isPt ? 'Motivo da parada' : 'Stop reason';
  String get avgTemp => isPt ? 'Temp. média' : 'Avg temp';
  String get maxTemp => isPt ? 'Temp. máxima' : 'Max temp';
  String get avgHumidity => isPt ? 'Umidade média' : 'Avg humidity';
  String get duration => isPt ? 'Duração' : 'Duration';
  String get device => isPt ? 'Dispositivo' : 'Device';
  String get tempChart => isPt ? 'Temperatura vs tempo' : 'Temperature vs time';
  String get humidityChart =>
      isPt ? 'Umidade vs tempo' : 'Humidity vs time';
  String get heaterChart =>
      isPt ? 'Aquecedor vs tempo' : 'Heater vs time';
  String get timeMinutes => isPt ? 'Tempo (min)' : 'Time (min)';
  String get noChartData =>
      isPt ? 'Sem dados de amostra para gráficos' : 'No sample chart data';
}

Future<Uint8List> exportCyclePdf(
  Map<String, dynamic> cycle, {
  Locale? locale,
}) async {
  final strings = _PdfStrings.fromLocale(locale);
  final samples =
      (cycle['samples'] as List?)?.cast<Map<String, dynamic>>() ?? [];

  final doc = pw.Document();
  doc.addPage(
    pw.MultiPage(
      pageFormat: PdfPageFormat.a4,
      build: (ctx) => [
        pw.Text(strings.title, style: pw.TextStyle(fontSize: 20)),
        pw.SizedBox(height: 12),
        pw.Text('${strings.material}: ${cycle['material_name'] ?? '-'}'),
        pw.Text(
          '${strings.targetTemp}: ${_fmtNum(cycle['target_temp_c'])} °C',
        ),
        pw.Text(
          '${strings.stopReason}: ${cycle['stop_reason'] ?? '-'}',
        ),
        pw.Text('${strings.avgTemp}: ${_fmtNum(cycle['avg_temp_c'])} °C'),
        pw.Text('${strings.maxTemp}: ${_fmtNum(cycle['max_temp_c'])} °C'),
        pw.Text(
          '${strings.avgHumidity}: ${_fmtNum(cycle['avg_humidity_pct'])} %RH',
        ),
        pw.Text(
          '${strings.duration}: ${_formatDuration(cycle)}',
        ),
        if (cycle['device_id'] != null)
          pw.Text('${strings.device}: ${cycle['device_id']}'),
        pw.SizedBox(height: 16),
        if (samples.isNotEmpty) ...[
          _buildChartSection(
            title: strings.tempChart,
            samples: samples,
            field: 'chamber_temp_c',
            color: PdfColors.orange,
            ySuffix: '°C',
            xLabel: strings.timeMinutes,
          ),
          pw.SizedBox(height: 12),
          _buildChartSection(
            title: strings.humidityChart,
            samples: samples,
            field: 'humidity_pct',
            color: PdfColors.blue,
            ySuffix: '%RH',
            xLabel: strings.timeMinutes,
          ),
          pw.SizedBox(height: 12),
          _buildChartSection(
            title: strings.heaterChart,
            samples: samples,
            field: 'heater_power_pct',
            color: PdfColors.red,
            ySuffix: '%',
            xLabel: strings.timeMinutes,
          ),
        ] else
          pw.Text(strings.noChartData),
      ],
    ),
  );
  return doc.save();
}

pw.Widget _buildChartSection({
  required String title,
  required List<Map<String, dynamic>> samples,
  required String field,
  required PdfColor color,
  required String ySuffix,
  required String xLabel,
}) {
  final points = _samplePoints(samples, field);
  if (points.length < 2) {
    return pw.Column(
      crossAxisAlignment: pw.CrossAxisAlignment.start,
      children: [
        pw.Text(title, style: pw.TextStyle(fontSize: 14)),
        pw.SizedBox(height: 4),
        pw.Text('-'),
      ],
    );
  }

  final xValues = points.map((p) => p.x).toList();
  final yValues = points.map((p) => p.y).toList();
  final xMin = xValues.reduce(math.min);
  final xMax = xValues.reduce(math.max);
  final yMin = yValues.reduce(math.min);
  final yMax = yValues.reduce(math.max);

  final xTicks = _axisTicks(xMin, xMax);
  final yTicks = _axisTicks(yMin, yMax);

  return pw.Column(
    crossAxisAlignment: pw.CrossAxisAlignment.start,
    children: [
      pw.Text(title, style: pw.TextStyle(fontSize: 14)),
      pw.SizedBox(height: 8),
      pw.SizedBox(
        height: 140,
        child: pw.Chart(
          title: pw.Text(title, style: const pw.TextStyle(fontSize: 10)),
          grid: pw.CartesianGrid(
            xAxis: pw.FixedAxis<double>(
              xTicks,
              format: (v) => v.toStringAsFixed(0),
              divisions: true,
            ),
            yAxis: pw.FixedAxis<double>(
              yTicks,
              format: (v) => v.toStringAsFixed(0),
              divisions: true,
            ),
          ),
          bottom: pw.Text(xLabel, style: const pw.TextStyle(fontSize: 8)),
          datasets: [
            pw.LineDataSet(
              legend: title,
              color: color,
              drawPoints: false,
              isCurved: true,
              data: points,
            ),
          ],
        ),
      ),
      pw.Text(
        'Min: ${yMin.toStringAsFixed(1)}$ySuffix · '
        'Max: ${yMax.toStringAsFixed(1)}$ySuffix · '
        'Points: ${points.length}',
        style: const pw.TextStyle(fontSize: 8),
      ),
    ],
  );
}

List<pw.PointChartValue> _samplePoints(
  List<Map<String, dynamic>> samples,
  String field,
) {
  final out = <pw.PointChartValue>[];
  for (final s in samples) {
    final x = s['t_sec'];
    final y = s[field];
    if (x is num && y is num) {
      out.add(pw.PointChartValue(x.toDouble() / 60, y.toDouble()));
    }
  }
  return out;
}

List<double> _axisTicks(double min, double max, {int count = 5}) {
  if (min == max) return [min];
  final step = (max - min) / (count - 1);
  return List.generate(count, (i) => min + step * i);
}

String _fmtNum(dynamic v) {
  if (v == null) return '-';
  if (v is num) return v.toStringAsFixed(1);
  return '$v';
}

String _formatDuration(Map<String, dynamic> cycle) {
  final started = cycle['started_at'] as String?;
  final ended = cycle['ended_at'] as String?;
  if (started != null && ended != null) {
    final sec = DateTime.parse(ended)
        .difference(DateTime.parse(started))
        .inSeconds;
    return _durationLabel(sec);
  }
  final samples = cycle['samples'];
  if (samples is List && samples.isNotEmpty) {
    final last = samples.last;
    if (last is Map) {
      final t = last['t_sec'];
      if (t is num) return _durationLabel(t.toInt());
    }
  }
  return '-';
}

String _durationLabel(int totalSeconds) {
  final hours = totalSeconds ~/ 3600;
  final minutes = (totalSeconds % 3600) ~/ 60;
  final seconds = totalSeconds % 60;
  if (hours > 0) {
    return '${hours}h ${minutes}m';
  }
  if (minutes > 0) {
    return '${minutes}m ${seconds}s';
  }
  return '${seconds}s';
}
