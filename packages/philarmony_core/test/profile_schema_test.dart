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
  final codec = ProfileCodec();

  test('schema constant loads from embedded installer-profile.schema.json', () {
    expect(ProfileCodec.schema['title'], contains('Installer Profile'));
    expect(ProfileCodec.schema['required'], contains('wifi'));
  });

  test('rejects invalid sensor enum', () {
    final json = codec.encode(
      DeviceProfile(
        wifi: WiFiConfig(ssid: 'lab', password: 'x'),
        sensors: [SensorConfig(role: 'temperature', sensor: 'dht22', gpioPin: 4)],
      ),
    );
    final bad = json.replaceAll('"dht22"', '"not-a-sensor"');
    expect(() => codec.decode(bad), throwsA(isA<FormatException>()));
  });

  test('rejects filament temp out of range', () {
    final profile = DeviceProfile(
      wifi: WiFiConfig(ssid: 'lab', password: 'x'),
      filamentProfiles: [
        FilamentProfile(
          id: 'hot',
          namePt: 'Quente',
          nameEn: 'Hot',
          targetTempC: 99,
          defaultDurationMin: 60,
          targetHumidityPct: 15,
          isBuiltin: false,
        ),
      ],
    );
    final json = codec.encode(profile);
    expect(() => codec.decode(json), throwsA(isA<FormatException>()));
  });

  test('rejects unknown top-level field (additionalProperties false)', () {
    final json = codec.encode(
      DeviceProfile(wifi: WiFiConfig(ssid: 'lab', password: 'x')),
    );
    final bad = json.replaceFirst('{', '{"extra":true,');
    expect(() => codec.decode(bad), throwsA(isA<FormatException>()));
  });

  test('round-trip valid profile', () {
    final profile = DeviceProfile(
      wifi: WiFiConfig(ssid: 'lab', password: 'secret'),
      sensors: [
        SensorConfig(role: 'temperature', sensor: 'dht22', gpioPin: 4),
      ],
    );
    final encoded = codec.encode(profile);
    final decoded = codec.decode(encoded);
    expect(decoded.wifi.ssid, 'lab');
    expect(decoded.sensors.first.sensor, 'dht22');
  });
}
