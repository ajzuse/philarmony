/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';
import 'dart:io';

import 'package:path_provider/path_provider.dart';

/// Exports cycle metadata + sample rows in one UTF-8 CSV file.
/// Sample rows use section=sample; metadata rows use section=meta.
Future<String> exportCycleCsv(
  Map<String, dynamic> cycle, {
  String? directory,
}) async {
  final dirPath = directory ?? (await getApplicationDocumentsDirectory()).path;
  final file = File('$dirPath/cycle_${cycle['id'] ?? 'export'}.csv');
  final buffer = StringBuffer('section,field,value\n');

  for (final e in cycle.entries) {
    if (e.key == 'samples') continue;
    buffer.writeln('meta,${e.key},${_csvCell(e.value)}');
  }

  final samples = cycle['samples'];
  if (samples is List) {
    for (final raw in samples) {
      if (raw is! Map) continue;
      final sample = Map<String, dynamic>.from(raw);
      for (final e in sample.entries) {
        buffer.writeln('sample,${e.key},${_csvCell(e.value)}');
      }
    }
  }

  await file.writeAsString(buffer.toString());
  return file.path;
}

String _csvCell(Object? value) {
  if (value == null) return '';
  final encoded = jsonEncode(value);
  if (encoded.startsWith('"') && encoded.endsWith('"')) {
    return encoded.substring(1, encoded.length - 1);
  }
  return encoded;
}
