import 'package:filament_dryer_control/device/fake_philarmony_ws_client.dart';
import 'package:filament_dryer_control/device/session_providers.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('start cycle exposes stop control state without pause affordance', () async {
    SharedPreferences.setMockInitialValues({});
    final fake = FakePhilarmonyWsClient();
    final container = ProviderContainer(
      overrides: [
        wsClientFactoryProvider.overrideWith((ref) => () => fake),
      ],
    );
    addTearDown(() async {
      try {
        await container.read(deviceSessionActionsProvider).disconnect();
      } catch (_) {}
      container.dispose();
    });

    await container.read(localStoreProvider.future);

    final device = KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1');
    await container.read(deviceSessionActionsProvider).connect(device);
    await container.read(deviceSessionActionsProvider).startCycle(
          const StartCycleRequest(targetTempC: 50, maxDurationMin: 60),
        );

    final status = container.read(deviceSessionProvider).status;
    expect(status?.status, DryerStatus.drying);
    expect(container.read(deviceSessionProvider).activeCycleId, isNotNull);
  });
}
