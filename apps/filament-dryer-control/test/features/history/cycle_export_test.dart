import 'dart:io';

import 'package:filament_dryer_control/data/app_database.dart';
import 'package:filament_dryer_control/data/drying_cycle_repository.dart';
import 'package:filament_dryer_control/features/history/cycle_csv_export.dart';
import 'package:filament_dryer_control/features/history/cycle_pdf_export.dart';
import 'package:filament_dryer_control/platform/cycle_file_export.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('export CSV and PDF for seeded cycle', () async {
    SharedPreferences.setMockInitialValues({});
    final db = AppDatabase.memory();
    final repo = DryingCycleRepository(db);
    final id = await repo.startCycle(
      deviceId: 'd1',
      request: const StartCycleRequest(targetTempC: 50, maxDurationMin: 60),
    );
    await repo.finalizeCycle(id, stopReason: 'user_stop');
    final cycle = (await repo.byId(id))!;

    final csvContent = buildCycleCsv(cycle);
    expect(csvContent, contains('section,field,value'));
    expect(csvContent, contains('meta,material_name'));

    final csvPath = await saveCycleCsv(
      cycle,
      directory: Directory.systemTemp.path,
    );
    expect(csvPath, contains('.csv'));
    expect(File(csvPath).existsSync(), isTrue);

    final pdf = await exportCyclePdf(cycle, locale: const Locale('en', 'US'));
    expect(pdf.isNotEmpty, isTrue);
    expect(String.fromCharCodes(pdf.take(4)), '%PDF');

    final pdfPt = await exportCyclePdf(cycle, locale: const Locale('pt', 'BR'));
    expect(pdfPt.isNotEmpty, isTrue);
  });
}
