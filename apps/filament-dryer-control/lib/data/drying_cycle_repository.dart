/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:drift/drift.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:uuid/uuid.dart';

import 'app_database.dart' as drift;

class DryingCycleRepository {
  DryingCycleRepository(this._db);

  final drift.AppDatabase _db;
  static const _uuid = Uuid();
  static const _maxStoredSamples = 2000;
  static const _downsampleEveryN = 5;

  final Map<String, List<Map<String, dynamic>>> _liveSamples = {};

  Future<List<Map<String, dynamic>>> list() async {
    final rows = await (_db.select(_db.dryingCycles)
          ..orderBy([(t) => OrderingTerm.desc(t.startTime)]))
        .get();
    final out = <Map<String, dynamic>>[];
    for (final row in rows) {
      out.add(await _toMap(row));
    }
    return out;
  }

  Future<String> startCycle({
    required String deviceId,
    required StartCycleRequest request,
    String? materialName,
  }) async {
    final id = _uuid.v4();
    await _db.into(_db.dryingCycles).insert(
          drift.DryingCyclesCompanion.insert(
            id: id,
            knownDeviceId: deviceId,
            profileId: Value(request.profileId),
            materialName: materialName ?? request.profileId ?? 'Manual',
            targetTempC: Value(request.targetTempC),
            maxDurationMin: Value(request.maxDurationMin),
            targetHumidityPct: Value(request.targetHumidityPct),
            startTime: DateTime.now().toUtc(),
            status: 'in_progress',
          ),
        );
    _liveSamples[id] = [];
    return id;
  }

  Future<void> appendSample(String cycleId, StatusSnapshot status) async {
    final samples = _liveSamples.putIfAbsent(cycleId, () => []);
    samples.add({
      't_sec': status.elapsedTimeSec,
      'chamber_temp_c': status.chamberTempC,
      'humidity_pct': status.humidityPct,
      'heater_power_pct': status.heaterPowerPct,
    });
    if (samples.length > 3600) {
      samples.removeRange(0, samples.length - 3600);
    }
  }

  Future<void> finalizeCycle(String cycleId, {required String stopReason}) async {
    final row = await (_db.select(_db.dryingCycles)
          ..where((t) => t.id.equals(cycleId)))
        .getSingleOrNull();
    if (row == null) return;

    final rawSamples = _liveSamples.remove(cycleId) ?? [];
    final downsampled = _downsample(rawSamples);
    final stats = _computeStats(downsampled);

    await (_db.update(_db.dryingCycles)..where((t) => t.id.equals(cycleId))).write(
      drift.DryingCyclesCompanion(
        endTime: Value(DateTime.now().toUtc()),
        stopReason: Value(stopReason),
        status: const Value('completed'),
        hasSamples: Value(downsampled.isNotEmpty),
        avgTempC: Value(stats.avgTempC),
        maxTempC: Value(stats.maxTempC),
        avgHumidityPct: Value(stats.avgHumidityPct),
      ),
    );

    if (downsampled.isNotEmpty) {
      await _db.batch((batch) {
        for (final s in downsampled) {
          batch.insert(
            _db.cycleSamples,
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

  Future<Map<String, dynamic>?> findInProgress(String deviceId) async {
    final row = await (_db.select(_db.dryingCycles)
          ..where(
            (t) =>
                t.knownDeviceId.equals(deviceId) & t.status.equals('in_progress'),
          ))
        .getSingleOrNull();
    if (row == null) return null;
    return _toMap(row);
  }

  Future<Map<String, dynamic>?> byId(String id) async {
    final row = await (_db.select(_db.dryingCycles)
          ..where((t) => t.id.equals(id)))
        .getSingleOrNull();
    if (row == null) return null;
    return _toMap(row);
  }

  Future<Map<String, dynamic>> _toMap(drift.DryingCycle row) async {
    List<Map<String, dynamic>> samples;
    if (row.status == 'in_progress') {
      samples = List<Map<String, dynamic>>.from(
        _liveSamples[row.id] ?? const [],
      );
    } else if (row.hasSamples) {
      final sampleRows = await (_db.select(_db.cycleSamples)
            ..where((t) => t.cycleId.equals(row.id))
            ..orderBy([(t) => OrderingTerm.asc(t.tSec)]))
          .get();
      samples = sampleRows
          .map(
            (s) => {
              't_sec': s.tSec,
              'chamber_temp_c': s.tempC,
              'humidity_pct': s.humidityPct,
              'heater_power_pct': s.heaterPct,
            },
          )
          .toList();
    } else {
      samples = [];
    }

    return {
      'id': row.id,
      'device_id': row.knownDeviceId,
      'profile_id': row.profileId,
      'material_name': row.materialName,
      'target_temp_c': row.targetTempC,
      'max_duration_min': row.maxDurationMin,
      'target_humidity_pct': row.targetHumidityPct,
      'started_at': row.startTime.toUtc().toIso8601String(),
      'ended_at': row.endTime?.toUtc().toIso8601String(),
      'status': row.status,
      'stop_reason': row.stopReason,
      'samples': samples,
      'has_samples': row.hasSamples || samples.isNotEmpty,
      'avg_temp_c': row.avgTempC,
      'max_temp_c': row.maxTempC,
      'avg_humidity_pct': row.avgHumidityPct,
    };
  }

  List<Map<String, dynamic>> _downsample(List<Map<String, dynamic>> samples) {
    if (samples.length <= _maxStoredSamples) {
      return List<Map<String, dynamic>>.from(samples);
    }
    final out = <Map<String, dynamic>>[];
    for (var i = 0; i < samples.length; i += _downsampleEveryN) {
      out.add(samples[i]);
      if (out.length >= _maxStoredSamples) break;
    }
    return out;
  }

  _CycleStats _computeStats(List<Map<String, dynamic>> samples) {
    double? avgTemp;
    double? maxTemp;
    double? avgHumidity;
    if (samples.isEmpty) {
      return const _CycleStats();
    }
    var tempSum = 0.0;
    var tempCount = 0;
    var humSum = 0.0;
    var humCount = 0;
    for (final s in samples) {
      final t = s['chamber_temp_c'];
      if (t is num) {
        final td = t.toDouble();
        tempSum += td;
        tempCount++;
        if (maxTemp == null || td > maxTemp) maxTemp = td;
      }
      final h = s['humidity_pct'];
      if (h is num) {
        humSum += h.toDouble();
        humCount++;
      }
    }
    if (tempCount > 0) avgTemp = tempSum / tempCount;
    if (humCount > 0) avgHumidity = humSum / humCount;
    return _CycleStats(
      avgTempC: avgTemp,
      maxTempC: maxTemp,
      avgHumidityPct: avgHumidity,
    );
  }
}

class _CycleStats {
  const _CycleStats({this.avgTempC, this.maxTempC, this.avgHumidityPct});

  final double? avgTempC;
  final double? maxTempC;
  final double? avgHumidityPct;
}
