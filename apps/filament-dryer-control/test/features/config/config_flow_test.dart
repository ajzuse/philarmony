import 'package:filament_dryer_control/device/fake_philarmony_ws_client.dart';
import 'package:filament_dryer_control/device/session_providers.dart';
import 'package:filament_dryer_control/features/config/config_controller.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('pin conflict is blocked before send', () async {
    SharedPreferences.setMockInitialValues({});
    final fake = FakePhilarmonyWsClient();
    final container = ProviderContainer(
      overrides: [wsClientProvider.overrideWithValue(fake)],
    );
    addTearDown(() {
      fake.dispose();
      container.dispose();
    });

    await container.read(localStoreProvider.future);
    await container.read(deviceSessionProvider.notifier).connect(
          KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1'),
        );

    final controller = container.read(configControllerProvider.notifier);
    controller.updateActuatorPin(0, 25);
    controller.updateActuatorPin(1, 25);
    await controller.save();

    final state = container.read(configControllerProvider);
    expect(state.saveState, ConfigSaveState.validationFailed);
    expect(state.validationErrors, isNotEmpty);
    expect(state.validationErrors.first, contains('Pin conflict'));
  });

  test('valid config receives saved ack from fake client', () async {
    SharedPreferences.setMockInitialValues({});
    final fake = FakePhilarmonyWsClient();
    final container = ProviderContainer(
      overrides: [wsClientProvider.overrideWithValue(fake)],
    );
    addTearDown(() {
      fake.dispose();
      container.dispose();
    });

    await container.read(localStoreProvider.future);
    await container.read(deviceSessionProvider.notifier).connect(
          KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1'),
        );

    final controller = container.read(configControllerProvider.notifier);
    await controller.save();

    await Future<void>.delayed(const Duration(milliseconds: 50));

    final state = container.read(configControllerProvider);
    expect(state.saveState, ConfigSaveState.saved);
    expect(fake.lastHardwareConfig, isNotNull);
  });
}
