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

import '../models/device_profile.dart';
import 'profile_codec.dart';

/// Export/import helpers — password always redacted on export (clarify R10).
class ProfileStore {
  ProfileStore({ProfileCodec? codec}) : _codec = codec ?? ProfileCodec();

  final ProfileCodec _codec;

  String exportJson(DeviceProfile profile) =>
      _codec.encode(profile, redactPassword: true);

  DeviceProfile importJson(String source) {
    final profile = _codec.decode(source);
    // Ensure imported password cannot be used for flash until re-entered.
    if (profile.wifi.password == ProfileCodec.redactedPassword ||
        profile.wifi.password.isEmpty) {
      profile.wifi.password = '';
    }
    return profile;
  }

  bool exportContainsRealPassword(String json) {
    final lower = json.toLowerCase();
    // Heuristic: after redaction password field is "***"
    return RegExp(r'"password"\s*:\s*"(?!\*\*\*")[^"]+"').hasMatch(lower);
  }
}
