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

    test('import always clears plaintext password from JSON', () {
      final store = ProfileStore();
      // Build a valid export then inject a real password string into the JSON.
      final base = store.exportJson(
        DeviceProfile(wifi: WiFiConfig(ssid: 'lab', password: 'original')),
      );
      final poisoned = base.replaceAll('"***"', '"should-never-survive"');
      expect(poisoned.contains('should-never-survive'), isTrue);
      final imported = store.importJson(poisoned);
      expect(imported.wifi.ssid, 'lab');
      expect(imported.wifi.password, isEmpty);
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

  group('PinValidator usable GPIOs', () {
    test('rejects out-of-range / non-usable GPIO', () {
      final profile = DeviceProfile(
        deviceModel: 'ESP32',
        pinMapping: PinMapping(heaterPwm: 99),
      );
      final r = PinValidator().validate(profile);
      expect(r.ok, isFalse);
      expect(r.errors.any((e) => e.contains('usable')), isTrue);
    });

    test('rejects flash-reserved GPIO that is also not usable', () {
      final profile = DeviceProfile(
        deviceModel: 'ESP32',
        pinMapping: PinMapping(heaterPwm: 6),
      );
      final r = PinValidator().validate(profile);
      expect(r.ok, isFalse);
    });
  });
}
