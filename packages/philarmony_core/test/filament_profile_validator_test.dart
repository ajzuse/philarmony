import 'package:philarmony_core/philarmony_core.dart';
import 'package:test/test.dart';

void main() {
  group('FilamentProfileValidator', () {
    test('accepts valid custom profile', () {
      final p = FilamentProfile(
        id: 'c1',
        namePt: 'Custom',
        nameEn: 'Custom',
        targetTempC: 55,
        defaultDurationMin: 120,
        targetHumidityPct: 15,
      );
      expect(FilamentProfileValidator.validate(p, customCount: 1), isEmpty);
    });

    test('rejects out-of-range temp', () {
      final p = FilamentProfile(
        id: 'c1',
        namePt: 'X',
        nameEn: 'X',
        targetTempC: 90,
        defaultDurationMin: 120,
        targetHumidityPct: 15,
      );
      expect(
        FilamentProfileValidator.validate(p, customCount: 1),
        contains(contains('Temperature')),
      );
    });

    test('rejects too many customs', () {
      final p = FilamentProfile(
        id: 'c1',
        namePt: 'X',
        nameEn: 'X',
        targetTempC: 50,
        defaultDurationMin: 60,
        targetHumidityPct: 15,
      );
      expect(
        FilamentProfileValidator.validate(p, customCount: 21),
        contains(contains('Max')),
      );
    });
  });

  group('NvsBinaryWriter', () {
    test('builds fixed-size NVS image with active page header and entry bitmap', () {
      final bytes = NvsBinaryWriter().build(
        namespace: 'filament_dryer',
        entries: {'wifi': '{"ssid":"t","password":"p"}'},
        sizeBytes: 0x5000,
      );
      expect(bytes.length, 0x5000);
      // ACTIVE state little-endian 0xFFFFFFFE
      expect(bytes[0], 0xfe);
      expect(bytes[1], 0xff);
      expect(bytes[2], 0xff);
      expect(bytes[3], 0xff);
      // First entry lives at offset 64 (after 32B header + 32B bitmap)
      expect(bytes[64], 0); // ns index for namespace entry
      expect(bytes[65], 0); // typeNamespace
    });
  });

  group('NvsConfigMapper preferences', () {
    test('emits filament_dryer keys', () {
      final m = NvsConfigMapper().toPreferencesStringEntries(
        DeviceProfile(
          wifi: WiFiConfig(ssid: 'home', password: 'secret'),
          pinMapping: PinMapping(heaterPwm: 25, exhaustFanPwm: 26, i2cSda: 21, i2cScl: 22),
        ),
      );
      expect(m.keys, containsAll(['wifi', 'sensor', 'actuator', 'display', 'profiles', 'objects', 'obj_control']));
      expect(m['wifi'], contains('home'));
      expect(m['wifi'], contains('secret'));
    });

    test('maps dual sensors to humidity_* when not integrated', () {
      final m = NvsConfigMapper().toPreferencesStringEntries(
        DeviceProfile(
          sensors: [
            SensorConfig(role: 'temperature', sensor: 'ds18b20', gpioPin: 4),
            SensorConfig(role: 'humidity', sensor: 'dht22', gpioPin: 5),
          ],
          pinMapping: PinMapping(heaterPwm: 25),
          wifi: WiFiConfig(ssid: 'x', password: 'y'),
        ),
      );
      expect(m['sensor'], contains('"is_integrated":false'));
      expect(m['sensor'], contains('"humidity_type":"dht22"'));
      expect(m['sensor'], contains('"humidity_gpio_pin":5'));
    });
  });
}
