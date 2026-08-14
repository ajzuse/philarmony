import 'package:filament_dryer_control/device/fake_philarmony_ws_client.dart';
import 'package:filament_dryer_control/device/session_providers.dart';
import 'package:filament_dryer_control/platform/background_session.dart';
import 'package:filament_dryer_control/platform/background_session_android.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('background session starts during drying status', () async {
    SharedPreferences.setMockInitialValues({});
    final fake = FakePhilarmonyWsClient();
    final bg = AndroidBackgroundSession();
    final container = ProviderContainer(
      overrides: [
        wsClientFactoryProvider.overrideWithValue(() => fake),
        backgroundSessionProvider.overrideWithValue(bg),
      ],
    );
    addTearDown(() async {
      try {
        await container.read(deviceSessionProvider.notifier).disconnect();
      } catch (_) {}
      container.dispose();
    });

    await container.read(localStoreProvider.future);

    await container.read(deviceSessionProvider.notifier).connect(
          KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1'),
        );
    await container.read(deviceSessionProvider.notifier).startCycle(
          const StartCycleRequest(targetTempC: 50, maxDurationMin: 60),
        );
    await Future<void>.delayed(const Duration(milliseconds: 200));

    expect(bg.isActive, isTrue);
  });
}
