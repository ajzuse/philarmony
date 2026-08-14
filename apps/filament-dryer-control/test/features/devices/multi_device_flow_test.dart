import 'package:filament_dryer_control/device/device_interfaces.dart';
import 'package:filament_dryer_control/device/fake_philarmony_ws_client.dart';
import 'package:filament_dryer_control/device/session_providers.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  late List<FakePhilarmonyWsClient> clients;

  ProviderContainer makeContainer({int maxSessions = 3}) {
      clients = [];
      SharedPreferences.setMockInitialValues({'max_sessions': maxSessions});
      return ProviderContainer(
        overrides: [
          wsClientFactoryProvider.overrideWith((ref) {
            return () {
              final client = FakePhilarmonyWsClient();
              clients.add(client);
              return client;
            };
          }),
        ],
      );
    }

    final device1 = KnownDevice(
      id: 'd1',
      nickname: 'Dryer A',
      host: '127.0.0.1',
    );
    final device2 = KnownDevice(
      id: 'd2',
      nickname: 'Dryer B',
      host: '127.0.0.2',
    );
    final device3 = KnownDevice(
      id: 'd3',
      nickname: 'Dryer C',
      host: '127.0.0.3',
    );
    final device4 = KnownDevice(
      id: 'd4',
      nickname: 'Dryer D',
      host: '127.0.0.4',
    );

    test('connects two devices and switches active session', () async {
      final container = makeContainer();
      addTearDown(() {
        for (final c in clients) {
          c.dispose();
        }
        container.dispose();
      });

      await container.read(localStoreProvider.future);
      final repo = await container.read(knownDeviceRepositoryProvider.future);
      await repo.upsert(device1);
      await repo.upsert(device2);

      final session = container.read(deviceSessionProvider.notifier);
      await session.connect(device1);
      await session.connect(device2);

      expect(container.read(sessionManagerProvider).sessions.length, 2);
      expect(
        container.read(deviceSessionProvider).activeDevice?.id,
        'd2',
      );

      await session.setActive('d1');
      expect(
        container.read(deviceSessionProvider).activeDevice?.id,
        'd1',
      );
      expect(
        container.read(deviceSessionProvider).connectionState,
        DeviceConnectionState.connected,
      );

      await session.disconnect();
      await session.disconnect();
    });

    test('enforces max concurrent sessions', () async {
      final container = makeContainer(maxSessions: 2);
      addTearDown(() {
        for (final c in clients) {
          c.dispose();
        }
        container.dispose();
      });

      await container.read(localStoreProvider.future);
      final session = container.read(deviceSessionProvider.notifier);
      await session.connect(device1);
      await session.connect(device2);

      expect(
        () => session.connect(device3),
        throwsA(isA<SessionLimitExceeded>()),
      );
      expect(container.read(sessionManagerProvider).sessions.length, 2);

      await session.disconnect('d1');
      await session.connect(device3);
      expect(container.read(sessionManagerProvider).sessions.length, 2);
      expect(
        container.read(deviceSessionProvider).activeDevice?.id,
        'd3',
      );

      await session.disconnect();
      await session.disconnect();
    });

    test('session manager tracks per-device online state', () async {
      final container = makeContainer();
      addTearDown(() {
        for (final c in clients) {
          c.dispose();
        }
        container.dispose();
      });

      await container.read(localStoreProvider.future);
      final session = container.read(deviceSessionProvider.notifier);
      await session.connect(device1);

      final manager = container.read(sessionManagerProvider);
      expect(manager.isOnline('d1'), isTrue);
      expect(manager.isOnline('d2'), isFalse);
      expect(
        manager.connectionStateFor('d1'),
        DeviceConnectionState.connected,
      );
      expect(
        manager.connectionStateFor('d2'),
        DeviceConnectionState.disconnected,
      );

      await session.disconnect();
    });

    test('device switcher state reflects active session nickname', () async {
      final container = makeContainer();
      addTearDown(() {
        for (final c in clients) {
          c.dispose();
        }
        container.dispose();
      });

      await container.read(localStoreProvider.future);
      final session = container.read(deviceSessionProvider.notifier);
      await session.connect(device1);
      await session.connect(device2);

      expect(
        container.read(sessionManagerProvider).activeSession?.device.nickname,
        'Dryer B',
      );

      await session.setActive('d1');
      expect(
        container.read(deviceSessionProvider).activeDevice?.nickname,
        'Dryer A',
      );

      await session.disconnect();
      await session.disconnect();
    });

    test('fourth device blocked when max is three', () async {
      final container = makeContainer(maxSessions: 3);
      addTearDown(() {
        for (final c in clients) {
          c.dispose();
        }
        container.dispose();
      });

      await container.read(localStoreProvider.future);
      final session = container.read(deviceSessionProvider.notifier);
      await session.connect(device1);
      await session.connect(device2);
      await session.connect(device3);

      expect(
        () => session.connect(device4),
        throwsA(isA<SessionLimitExceeded>()),
      );

      await session.disconnect();
      await session.disconnect();
      await session.disconnect();
    });
}
