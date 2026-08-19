/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

import 'package:drift/drift.dart';
import 'package:philarmony_core/philarmony_core.dart' as core;

import 'app_database.dart' as drift;

class KnownDeviceRepository {
  KnownDeviceRepository(this._db);

  final drift.AppDatabase _db;

  Future<List<core.KnownDevice>> list() async {
    final rows = await _db.select(_db.knownDevices).get();
    return rows.map(_fromRow).toList();
  }

  Future<core.KnownDevice?> findByHostPort(String host, int port) async {
    final row = await (_db.select(_db.knownDevices)
          ..where((t) => t.host.equals(host) & t.port.equals(port)))
        .getSingleOrNull();
    return row == null ? null : _fromRow(row);
  }

  Future<void> upsert(core.KnownDevice device) async {
    final existing = await findByHostPort(device.host, device.port);
    final id = existing?.id ?? device.id;
    final toSave = existing != null && existing.id != device.id
        ? _withId(device, existing.id)
        : (device.id == id ? device : _withId(device, id));

    await _db.into(_db.knownDevices).insertOnConflictUpdate(
          drift.KnownDevicesCompanion(
            id: Value(id),
            nickname: Value(toSave.nickname),
            host: Value(toSave.host),
            port: Value(toSave.port),
            path: Value(toSave.path),
            deviceModel: Value(toSave.deviceModel),
            firmwareVersion: Value(toSave.firmwareVersion),
            lastSeen: Value(toSave.lastSeen),
            lastConnected: Value(toSave.lastConnected),
            autoConnect: Value(toSave.autoConnect),
            notificationSettingsJson:
                Value(jsonEncode(toSave.notificationSettings)),
          ),
        );
  }

  Future<void> remove(String id) async {
    await (_db.delete(_db.knownDevices)..where((t) => t.id.equals(id))).go();
  }

  Future<core.KnownDevice?> autoConnectCandidate() async {
    final devices = await list();
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

  core.KnownDevice _withId(core.KnownDevice device, String id) {
    return core.KnownDevice(
      id: id,
      nickname: device.nickname,
      host: device.host,
      port: device.port,
      path: device.path,
      deviceModel: device.deviceModel,
      firmwareVersion: device.firmwareVersion,
      lastSeen: device.lastSeen,
      lastConnected: device.lastConnected,
      autoConnect: device.autoConnect,
      notificationSettings: device.notificationSettings,
    );
  }

  core.KnownDevice _fromRow(drift.KnownDevice row) {
    Map<String, bool> notificationSettings = {};
    try {
      final decoded = jsonDecode(row.notificationSettingsJson);
      if (decoded is Map) {
        notificationSettings = decoded.map(
          (k, v) => MapEntry(k.toString(), v == true),
        );
      }
    } catch (_) {}

    return core.KnownDevice(
      id: row.id,
      nickname: row.nickname,
      host: row.host,
      port: row.port,
      path: row.path,
      deviceModel: row.deviceModel,
      firmwareVersion: row.firmwareVersion,
      lastSeen: row.lastSeen,
      lastConnected: row.lastConnected,
      autoConnect: row.autoConnect,
      notificationSettings: notificationSettings,
    );
  }
}
