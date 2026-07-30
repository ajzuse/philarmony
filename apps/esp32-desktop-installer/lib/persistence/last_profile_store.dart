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

import 'dart:io';

import 'package:path/path.dart' as p;
import 'package:path_provider/path_provider.dart';
import 'package:philarmony_core/philarmony_core.dart';

/// Persists last successful DeviceProfile without WiFi password (R9/R10).
class LastProfileStore {
  final ProfileStore _store = ProfileStore();

  Future<File> _file() async {
    final dir = await getApplicationSupportDirectory();
    return File(p.join(dir.path, 'last_profile.json'));
  }

  Future<void> save(DeviceProfile profile) async {
    final f = await _file();
    await f.parent.create(recursive: true);
    await f.writeAsString(_store.exportJson(profile));
  }

  Future<DeviceProfile?> load() async {
    final f = await _file();
    if (!await f.exists()) return null;
    return _store.importJson(await f.readAsString());
  }
}

/// Tiny helper for unit tests without path_provider.
class InMemoryLastProfileStore {
  String? _json;
  final ProfileStore _store = ProfileStore();

  Future<void> save(DeviceProfile profile) async {
    _json = _store.exportJson(profile);
  }

  Future<DeviceProfile?> load() async {
    if (_json == null) return null;
    return _store.importJson(_json!);
  }

  String? peekRaw() => _json;
}
