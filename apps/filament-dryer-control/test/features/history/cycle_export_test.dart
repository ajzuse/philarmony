import 'dart:io';

import 'package:filament_dryer_control/data/app_preferences.dart';
import 'package:filament_dryer_control/data/drying_cycle_repository.dart';
import 'package:filament_dryer_control/features/history/cycle_csv_export.dart';
import 'package:filament_dryer_control/features/history/cycle_pdf_export.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('export CSV and PDF for seeded cycle', () async {
    SharedPreferences.setMockInitialValues({});
    final store = LocalStore(await SharedPreferences.getInstance());
    final repo = DryingCycleRepository(store);
    final id = await repo.startCycle(
      deviceId: 'd1',
      request: const StartCycleRequest(targetTempC: 50, maxDurationMin: 60),
    );
    await repo.finalizeCycle(id, stopReason: 'user_stop');
    final cycle = repo.byId(id)!;

    final csvPath = await exportCycleCsv(
      cycle,
      directory: Directory.systemTemp.path,
    );
    expect(csvPath, contains('.csv'));
    expect(File(csvPath).existsSync(), isTrue);

    final pdf = await exportCyclePdf(cycle);
    expect(pdf.isNotEmpty, isTrue);
    expect(String.fromCharCodes(pdf.take(4)), '%PDF');
  });
}
