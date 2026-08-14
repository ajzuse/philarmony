/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:philarmony_core/philarmony_core.dart';

import 'app_preferences.dart';

class KnownDeviceRepository {
  KnownDeviceRepository(this._store);

  final LocalStore _store;

  List<KnownDevice> list() => _store.loadDevices();

  Future<void> upsert(KnownDevice device) async {
    final devices = list();
    final idx = devices.indexWhere((d) => d.id == device.id);
    if (idx >= 0) {
      devices[idx] = device;
    } else {
      devices.add(device);
    }
    await _store.saveDevices(devices);
  }

  Future<void> remove(String id) async {
    final devices = list()..removeWhere((d) => d.id == id);
    await _store.saveDevices(devices);
  }

  KnownDevice? autoConnectCandidate() {
    final devices = list();
    final flagged = devices.where((d) => d.autoConnect).toList();
    if (flagged.isNotEmpty) {
      flagged.sort((a, b) =>
          (b.lastConnected ?? DateTime.fromMillisecondsSinceEpoch(0))
              .compareTo(a.lastConnected ?? DateTime.fromMillisecondsSinceEpoch(0)));
      return flagged.first;
    }
    if (devices.isEmpty) return null;
    devices.sort((a, b) =>
        (b.lastConnected ?? DateTime.fromMillisecondsSinceEpoch(0))
            .compareTo(a.lastConnected ?? DateTime.fromMillisecondsSinceEpoch(0)));
    return devices.first;
  }
}
