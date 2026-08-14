/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:philarmony_core/philarmony_core.dart';
import 'package:uuid/uuid.dart';

import 'app_preferences.dart';

class DryingCycleRepository {
  DryingCycleRepository(this._store);

  final LocalStore _store;
  static const _uuid = Uuid();
  static const _maxStoredSamples = 2000;
  static const _downsampleEveryN = 5;

  List<Map<String, dynamic>> list() => _store.loadCycles();

  Future<String> startCycle({
    required String deviceId,
    required StartCycleRequest request,
    String? materialName,
  }) async {
    final id = _uuid.v4();
    final cycles = list();
    cycles.insert(0, {
      'id': id,
      'device_id': deviceId,
      'profile_id': request.profileId,
      'material_name': materialName ?? request.profileId ?? 'Manual',
      'target_temp_c': request.targetTempC,
      'max_duration_min': request.maxDurationMin,
      'target_humidity_pct': request.targetHumidityPct,
      'started_at': DateTime.now().toUtc().toIso8601String(),
      'status': 'in_progress',
      'samples': <Map<String, dynamic>>[],
    });
    await _store.saveCycles(cycles);
    return id;
  }

  Future<void> appendSample(String cycleId, StatusSnapshot status) async {
    final cycles = list();
    final idx = cycles.indexWhere((c) => c['id'] == cycleId);
    if (idx < 0) return;
    final samples = (cycles[idx]['samples'] as List).cast<Map<String, dynamic>>();
    samples.add({
      't_sec': status.elapsedTimeSec,
      'chamber_temp_c': status.chamberTempC,
      'humidity_pct': status.humidityPct,
      'heater_power_pct': status.heaterPowerPct,
    });
    if (samples.length > 3600) {
      samples.removeRange(0, samples.length - 3600);
    }
    cycles[idx]['samples'] = samples;
    await _store.saveCycles(cycles);
  }

  Future<void> finalizeCycle(String cycleId, {required String stopReason}) async {
    final cycles = list();
    final idx = cycles.indexWhere((c) => c['id'] == cycleId);
    if (idx < 0) return;

    final rawSamples =
        (cycles[idx]['samples'] as List?)?.cast<Map<String, dynamic>>() ?? [];
    final downsampled = _downsample(rawSamples);
    final stats = _computeStats(downsampled);

    cycles[idx]
      ..['status'] = 'completed'
      ..['ended_at'] = DateTime.now().toUtc().toIso8601String()
      ..['stop_reason'] = stopReason
      ..['samples'] = downsampled
      ..['has_samples'] = downsampled.isNotEmpty
      ..['avg_temp_c'] = stats.avgTempC
      ..['max_temp_c'] = stats.maxTempC
      ..['avg_humidity_pct'] = stats.avgHumidityPct;

    await _store.saveCycles(cycles);
  }

  Map<String, dynamic>? findInProgress(String deviceId) {
    for (final c in list()) {
      if (c['device_id'] == deviceId && c['status'] == 'in_progress') {
        return c;
      }
    }
    return null;
  }

  Map<String, dynamic>? byId(String id) {
    for (final c in list()) {
      if (c['id'] == id) return c;
    }
    return null;
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
