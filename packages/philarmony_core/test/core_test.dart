import 'package:philarmony_core/philarmony_core.dart';
import 'package:test/test.dart';

void main() {
  group('PinValidator', () {
    test('rejects duplicate pins', () {
      final profile = DeviceProfile(
        pinMapping: PinMapping(heaterPwm: 25, exhaustFanPwm: 25),
      );
      final r = PinValidator().validate(profile);
      expect(r.ok, isFalse);
      expect(r.errors.any((e) => e.contains('Duplicate')), isTrue);
    });

    test('rejects ESP32 input-only GPIO for heater', () {
      final profile = DeviceProfile(
        deviceModel: 'ESP32',
        pinMapping: PinMapping(heaterPwm: 34),
      );
      final r = PinValidator().validate(profile);
      expect(r.ok, isFalse);
    });

    test('accepts valid classic mapping', () {
      final profile = DeviceProfile(
        pinMapping: PinMapping(
          heaterPwm: 25,
          exhaustFanPwm: 26,
          i2cSda: 21,
          i2cScl: 22,
        ),
      );
      expect(PinValidator().validate(profile).ok, isTrue);
    });
  });

  group('ProfileStore', () {
    test('export never contains real password', () {
      final profile = DeviceProfile(
        wifi: WiFiConfig(ssid: 'home', password: 'secret-pass'),
      );
      final store = ProfileStore();
      final json = store.exportJson(profile);
      expect(json.contains('secret-pass'), isFalse);
      expect(json.contains('"***"'), isTrue);
      expect(store.exportContainsRealPassword(json), isFalse);
    });

    test('import clears redacted password', () {
      final store = ProfileStore();
      final exported = store.exportJson(
        DeviceProfile(wifi: WiFiConfig(ssid: 'x', password: 'abc')),
      );
      final imported = store.importJson(exported);
      expect(imported.wifi.ssid, 'x');
      expect(imported.wifi.hasPassword, isFalse);
    });

    test('import rejects missing required schema fields', () {
      final store = ProfileStore();
      expect(
        () => store.importJson('{"profile_version":1}'),
        throwsA(isA<FormatException>()),
      );
    });
  });
}
