/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

/// Builds UTF-8 CSV content for a cycle.
/// Sample rows use section=sample; metadata rows use section=meta.
String buildCycleCsv(Map<String, dynamic> cycle) {
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

  return buffer.toString();
}

String _csvCell(Object? value) {
  if (value == null) return '';
  final encoded = jsonEncode(value);
  if (encoded.startsWith('"') && encoded.endsWith('"')) {
    return encoded.substring(1, encoded.length - 1);
  }
  return encoded;
}
