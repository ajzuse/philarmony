import 'package:filament_dryer_control/device/device_interfaces.dart';
import 'package:filament_dryer_control/device/fake_philarmony_ws_client.dart';
import 'package:filament_dryer_control/device/session_providers.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('dashboard session connects with fake client', () async {
    SharedPreferences.setMockInitialValues({});
    final fake = FakePhilarmonyWsClient();
    final container = ProviderContainer(
      overrides: [wsClientFactoryProvider.overrideWithValue(() => fake)],
    );
    addTearDown(() async {
      try {
        await container.read(deviceSessionActionsProvider).disconnect();
      } catch (_) {}
      container.dispose();
    });

    await container.read(localStoreProvider.future);
    await container.read(deviceSessionActionsProvider).connect(
          KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1'),
        );

    expect(
      container.read(deviceSessionProvider).connectionState,
      DeviceConnectionState.connected,
    );
    expect(container.read(deviceSessionProvider).status?.status, DryerStatus.idle);
  });
}
