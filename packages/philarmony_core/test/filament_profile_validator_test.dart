/*
 * Philarmony Filament Dryer — Desktop Installer / Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

    test('round-trips namespace string entries (Preferences parity)', () {
      final entries = {
        'wifi': '{"ssid":"lab","password":"secret","ip":"192.168.1.50"}',
        'sensor': '{"type":"sht31","parameters":{"offset":0.5}}',
      };
      final bytes = NvsBinaryWriter().build(
        namespace: 'filament_dryer',
        entries: entries,
        sizeBytes: 0x5000,
      );
      final decoded = NvsBinaryWriter.readStringEntries(bytes);
      expect(decoded.keys, contains('filament_dryer'));
      expect(decoded['filament_dryer']!['wifi'], entries['wifi']);
      expect(decoded['filament_dryer']!['sensor'], entries['sensor']);
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

    test('injects advanced sensor JSON parameters into Preferences sensor blob', () {
      final m = NvsConfigMapper().toPreferencesStringEntries(
        DeviceProfile(
          sensors: [
            SensorConfig(
              role: 'temperature',
              sensor: 'ntc',
              gpioPin: 34,
              parameters: {'offset': 1.5, 'scale': 0.98, 'beta': 3950},
            ),
            SensorConfig(
              role: 'humidity',
              sensor: 'dht22',
              gpioPin: 5,
              parameters: {'offset': -0.2, 'scale': 1.01},
            ),
          ],
          pinMapping: PinMapping(heaterPwm: 25),
          wifi: WiFiConfig(ssid: 'x', password: 'y'),
        ),
      );
      expect(m['sensor'], contains('"parameters"'));
      expect(m['sensor'], contains('"beta":3950'));
      expect(m['sensor'], contains('"humidity_parameters"'));
      expect(m['sensor'], contains('"temperature_offset":1.5'));
      expect(m['sensor'], contains('"humidity_offset":-0.2'));
    });

    test('emits flat static IP fields for firmware WifiConfig', () {
      final wifi = NvsConfigMapper().toWifiNvs(
        DeviceProfile(
          wifi: WiFiConfig(
            ssid: 'lab',
            password: 'p',
            staticIp: {
              'ip': '192.168.1.50',
              'gateway': '192.168.1.1',
              'netmask': '255.255.255.0',
              'dns': '1.1.1.1',
            },
          ),
        ),
      );
      expect(wifi['ip'], '192.168.1.50');
      expect(wifi['gateway'], '192.168.1.1');
      expect(wifi['use_static_ip'], isTrue);
      expect(wifi['static_ip'], isA<Map>());
    });

    test('validateStaticIp rejects bad IPv4', () {
      final errors = NvsConfigMapper.validateStaticIp({
        'ip': '999.1.1.1',
        'gateway': '192.168.1.1',
        'netmask': '255.255.255.0',
        'dns': '',
      });
      expect(errors, isNotEmpty);
    });
  });
}
