import 'package:filament_dryer_control/data/pending_command_repository.dart';
import 'package:filament_dryer_control/device/fake_philarmony_ws_client.dart';
import 'package:filament_dryer_control/device/session_deps.dart';
import 'package:filament_dryer_control/device/session_providers.dart';
import 'package:filament_dryer_control/features/profiles/profile_sync_service.dart';
import 'package:filament_dryer_control/features/profiles/profiles_providers.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  group('profiles flow', () {
    late FakePhilarmonyWsClient fake;

    ProviderContainer newContainer() {
      fake = FakePhilarmonyWsClient();
      return ProviderContainer(
        overrides: [
          wsClientFactoryProvider.overrideWithValue(() => fake),
        ],
      );
    }

    test('lists built-in profiles from device when connected', () async {
      SharedPreferences.setMockInitialValues({});
      final container = newContainer();
      addTearDown(() async {
        try {
          await container.read(deviceSessionActionsProvider).disconnect();
        } catch (_) {}
        container.dispose();
      });

      final device = KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1');
      await container.read(deviceSessionActionsProvider).connect(device);

      final profiles = await container.read(profilesListProvider.future);
      expect(profiles.length, FilamentProfile.builtins().length);
      expect(profiles.any((p) => p.id == 'pla'), isTrue);
      expect(profiles.every((p) => p.isBuiltin), isTrue);
    });

    test('creates custom profile online and lists it', () async {
      SharedPreferences.setMockInitialValues({});
      final container = newContainer();
      addTearDown(() async {
        try {
          await container.read(deviceSessionActionsProvider).disconnect();
        } catch (_) {}
        container.dispose();
      });

      final device = KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1');
      await container.read(deviceSessionActionsProvider).connect(device);

      final svc = await container.read(profileSyncServiceProvider.future);
      await svc.createProfile(
        FilamentProfile(
          id: 'custom-test',
          namePt: 'Teste',
          nameEn: 'Test',
          targetTempC: 55,
          defaultDurationMin: 120,
          targetHumidityPct: 12,
        ),
      );

      final profiles = await container.read(profilesListProvider.future);
      expect(profiles.any((p) => p.id.startsWith('custom-')), isTrue);
    });

    test('queues offline profile edit and flushes on reconnect', () async {
      SharedPreferences.setMockInitialValues({});
      final container = newContainer();
      addTearDown(() async {
        try {
          await container.read(deviceSessionActionsProvider).disconnect();
        } catch (_) {}
        container.dispose();
      });

      final device = KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1');
      final pending = await container.read(pendingCommandRepositoryProvider.future);
      await pending.enqueue(
        deviceId: device.id,
        topic: 'config/profiles/create',
        payload: {
          'name_pt': 'Offline',
          'name_en': 'Offline',
          'target_temp_c': 52,
          'default_duration_min': 90,
          'target_humidity_pct': 14,
        },
      );

      expect(await pending.listQueued(device.id), hasLength(1));

      await container.read(deviceSessionActionsProvider).connect(device);
      await Future<void>.delayed(const Duration(milliseconds: 50));

      expect(await pending.listQueued(device.id), isEmpty);
    });

    test('start cycle with profile_id only when using preset unchanged', () async {
      SharedPreferences.setMockInitialValues({});
      final container = newContainer();
      addTearDown(() async {
        try {
          await container.read(deviceSessionActionsProvider).disconnect();
        } catch (_) {}
        container.dispose();
      });

      final device = KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1');
      await container.read(deviceSessionActionsProvider).connect(device);

      const request = StartCycleRequest(profileId: 'pla');
      expect(CycleCommandValidator.validateStart(request), isEmpty);

      await container.read(deviceSessionActionsProvider).startCycle(request);

      final repo = await container.read(dryingCycleRepositoryProvider.future);
      final cycles = await repo.list();
      final cycle = cycles.first;
      expect(cycle['profile_id'], 'pla');
    });

    test('built-in profiles cannot be deleted', () async {
      SharedPreferences.setMockInitialValues({});
      final container = newContainer();
      addTearDown(() async {
        try {
          await container.read(deviceSessionActionsProvider).disconnect();
        } catch (_) {}
        container.dispose();
      });

      final device = KnownDevice(id: 'd1', nickname: 'Dryer', host: '127.0.0.1');
      await container.read(deviceSessionActionsProvider).connect(device);

      final svc = await container.read(profileSyncServiceProvider.future);
      expect(
        () => svc.deleteProfile('pla'),
        throwsA(isA<ArgumentError>()),
      );
    });
  });
}
