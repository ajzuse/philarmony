import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'package:filament_dryer_control/app.dart';
import 'package:filament_dryer_control/device/fake_philarmony_ws_client.dart';
import 'package:filament_dryer_control/device/session_providers.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('PhilarmonyControlApp builds', (tester) async {
    SharedPreferences.setMockInitialValues({});
    final fake = FakePhilarmonyWsClient();
    await tester.pumpWidget(
      ProviderScope(
        overrides: [
          wsClientFactoryProvider.overrideWithValue(() => fake),
        ],
        child: const PhilarmonyControlApp(),
      ),
    );
    await tester.pump();
    await tester.pump(const Duration(milliseconds: 100));
  });
}
