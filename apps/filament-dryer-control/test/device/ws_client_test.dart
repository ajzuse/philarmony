import 'package:filament_dryer_control/device/device_interfaces.dart';
import 'package:filament_dryer_control/device/fake_philarmony_ws_client.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';

void main() {
  test('FakePhilarmonyWsClient connects and streams status', () async {
    final client = FakePhilarmonyWsClient();
    final states = <DeviceConnectionState>[];
    final statuses = <StatusSnapshot>[];

    client.connectionStates.listen(states.add);
    client.statusStream.listen(statuses.add);

    await client.connect(
      KnownDevice(id: '1', nickname: 'Test', host: '127.0.0.1'),
    );

    await Future<void>.delayed(const Duration(milliseconds: 100));

    expect(states, contains(DeviceConnectionState.connected));
    expect(statuses, isNotEmpty);
    expect(client.state, DeviceConnectionState.connected);

    await client.disconnect();
    expect(client.state, DeviceConnectionState.disconnected);
    client.dispose();
  });

  test('FakePhilarmonyWsClient start/stop cycle updates status', () async {
    final client = FakePhilarmonyWsClient();
    await client.connect(
      KnownDevice(id: '1', nickname: 'Test', host: '127.0.0.1'),
    );

    await client.startCycle(
      const StartCycleRequest(targetTempC: 50, maxDurationMin: 60),
    );
    await Future<void>.delayed(const Duration(milliseconds: 50));
    expect(client.lastStatus?.status, DryerStatus.drying);

    await client.stopCycle();
    await Future<void>.delayed(const Duration(milliseconds: 50));
    expect(client.lastStatus?.status, DryerStatus.stopped);

    client.dispose();
  });
}
