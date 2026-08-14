import 'package:filament_dryer_control/data/app_database.dart';
import 'package:filament_dryer_control/data/drying_cycle_repository.dart';
import 'package:filament_dryer_control/device/session_deps.dart';
import 'package:filament_dryer_control/features/history/history_page.dart';
import 'package:filament_dryer_control/l10n/app_localizations.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('history list shows seeded completed cycle offline', (tester) async {
    SharedPreferences.setMockInitialValues({});
    final db = AppDatabase.memory();
    final repo = DryingCycleRepository(db);
    final id = await repo.startCycle(
      deviceId: 'd1',
      request: const StartCycleRequest(targetTempC: 55, maxDurationMin: 90),
      materialName: 'PLA',
    );
    await repo.finalizeCycle(id, stopReason: 'completed');

    await tester.pumpWidget(
      ProviderScope(
        overrides: [
          appDatabaseProvider.overrideWithValue(db),
          storeMigrationProvider.overrideWith((ref) async {}),
        ],
        child: const MaterialApp(
          localizationsDelegates: AppLocalizations.localizationsDelegates,
          supportedLocales: AppLocalizations.supportedLocales,
          home: Scaffold(body: HistoryPage()),
        ),
      ),
    );
    await tester.pumpAndSettle();

    expect(find.text('PLA'), findsWidgets);
    expect(find.textContaining('55.0'), findsWidgets);
  });
}
