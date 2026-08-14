/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

import 'package:drift/drift.dart';
import 'package:philarmony_core/philarmony_core.dart' as core;
import 'package:shared_preferences/shared_preferences.dart';

import 'app_database.dart' as drift;
import 'app_preferences.dart';

const driftMigratedKey = 'drift_migrated';

Future<void> migrateStoreIfNeeded(
  SharedPreferences prefs,
  drift.AppDatabase db,
) async {
  if (prefs.getBool(driftMigratedKey) == true) return;

  final store = LocalStore(prefs);

  for (final device in store.loadDevices()) {
    final existing = await (db.select(db.knownDevices)
          ..where((t) => t.host.equals(device.host) & t.port.equals(device.port)))
        .getSingleOrNull();
    final id = existing?.id ?? device.id;
    await db.into(db.knownDevices).insertOnConflictUpdate(
          _deviceCompanion(id, device),
        );
  }

  for (final cycle in store.loadCycles()) {
    final cycleId = cycle['id'] as String;
    final existing = await (db.select(db.dryingCycles)
          ..where((t) => t.id.equals(cycleId)))
        .getSingleOrNull();
    if (existing != null) continue;

    final samples =
        (cycle['samples'] as List?)?.cast<Map<String, dynamic>>() ?? [];
    final downsampled = samples.length > 2000
        ? [for (var i = 0; i < samples.length; i += 5) samples[i]]
        : samples;

    await db.into(db.dryingCycles).insert(
          drift.DryingCyclesCompanion.insert(
            id: cycleId,
            knownDeviceId: cycle['device_id'] as String,
            profileId: Value(cycle['profile_id'] as String?),
            materialName: cycle['material_name'] as String? ?? 'Manual',
            targetTempC: Value((cycle['target_temp_c'] as num?)?.toDouble()),
            maxDurationMin: Value((cycle['max_duration_min'] as num?)?.toInt()),
            targetHumidityPct:
                Value((cycle['target_humidity_pct'] as num?)?.toDouble()),
            startTime: DateTime.parse(cycle['started_at'] as String),
            endTime: cycle['ended_at'] != null
                ? Value(DateTime.parse(cycle['ended_at'] as String))
                : const Value.absent(),
            avgTempC: Value((cycle['avg_temp_c'] as num?)?.toDouble()),
            maxTempC: Value((cycle['max_temp_c'] as num?)?.toDouble()),
            avgHumidityPct:
                Value((cycle['avg_humidity_pct'] as num?)?.toDouble()),
            stopReason: Value(cycle['stop_reason'] as String?),
            status: cycle['status'] as String? ?? 'completed',
            hasSamples: Value(downsampled.isNotEmpty),
          ),
        );

    if (downsampled.isNotEmpty) {
      await db.batch((batch) {
        for (final s in downsampled) {
          batch.insert(
            db.cycleSamples,
            drift.CycleSamplesCompanion.insert(
              cycleId: cycleId,
              tSec: (s['t_sec'] as num?)?.toInt() ?? 0,
              tempC: Value((s['chamber_temp_c'] as num?)?.toDouble()),
              humidityPct: Value((s['humidity_pct'] as num?)?.toDouble()),
              heaterPct: Value(
                (s['heater_power_pct'] as num?)?.toDouble() ?? 0,
              ),
            ),
          );
        }
      });
    }
  }

  for (final raw in store.loadPendingCommands()) {
    final id = raw['id'] as String;
    final existing = await (db.select(db.pendingCommands)
          ..where((t) => t.id.equals(id)))
        .getSingleOrNull();
    if (existing != null) continue;

    await db.into(db.pendingCommands).insert(
          drift.PendingCommandsCompanion.insert(
            id: id,
            knownDeviceId: raw['known_device_id'] as String,
            topic: raw['topic'] as String,
            payloadJson: raw['payload_json'] as String,
            createdAt: DateTime.parse(raw['created_at'] as String),
            status: raw['status'] as String? ?? 'queued',
            lastError: Value(raw['last_error'] as String?),
          ),
        );
  }

  await prefs.setBool(driftMigratedKey, true);
}

drift.KnownDevicesCompanion _deviceCompanion(String id, core.KnownDevice device) {
  return drift.KnownDevicesCompanion.insert(
    id: id,
    nickname: device.nickname,
    host: device.host,
    port: Value(device.port),
    path: Value(device.path),
    deviceModel: Value(device.deviceModel),
    firmwareVersion: Value(device.firmwareVersion),
    lastSeen: Value(device.lastSeen),
    lastConnected: Value(device.lastConnected),
    autoConnect: Value(device.autoConnect),
    notificationSettingsJson: Value(jsonEncode(device.notificationSettings)),
  );
}
