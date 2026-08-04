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

import 'package:esp32_desktop_installer/persistence/last_profile_store.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';

void main() {
  test('InMemoryLastProfileStore redacts password', () async {
    final store = InMemoryLastProfileStore();
    await store.save(
      DeviceProfile(wifi: WiFiConfig(ssid: 'net', password: 'super-secret')),
    );
    final raw = store.peekRaw()!;
    expect(raw.contains('super-secret'), isFalse);
    expect(raw.contains('***'), isTrue);
    final loaded = await store.load();
    expect(loaded!.wifi.ssid, 'net');
    expect(loaded.wifi.hasPassword, isFalse);
  });
}
