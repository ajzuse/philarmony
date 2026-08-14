/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';

import '../features/history/cycle_csv_export.dart';
import '../features/history/cycle_pdf_export.dart';

/// Writes cycle CSV to disk and returns the file path.
Future<String> saveCycleCsv(
  Map<String, dynamic> cycle, {
  String? directory,
}) async {
  final dirPath = directory ?? (await getApplicationDocumentsDirectory()).path;
  final file = File('$dirPath/cycle_${cycle['id'] ?? 'export'}.csv');
  await file.writeAsString(buildCycleCsv(cycle));
  return file.path;
}

/// Generates cycle PDF bytes (caller may share or save).
Future<Uint8List> buildCyclePdfFile(
  Map<String, dynamic> cycle, {
  Locale? locale,
}) {
  return exportCyclePdf(cycle, locale: locale);
}

/// Writes cycle PDF to disk and returns the file path.
Future<String> saveCyclePdf(
  Map<String, dynamic> cycle, {
  String? directory,
  Locale? locale,
}) async {
  final dirPath = directory ?? (await getApplicationDocumentsDirectory()).path;
  final file = File('$dirPath/cycle_${cycle['id'] ?? 'export'}.pdf');
  final bytes = await exportCyclePdf(cycle, locale: locale);
  await file.writeAsBytes(bytes);
  return file.path;
}
