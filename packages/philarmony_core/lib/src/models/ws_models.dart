/*
 * Philarmony Filament Dryer — Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

class WsEnvelope {
  const WsEnvelope({required this.topic, required this.payload});

  final String topic;
  final Map<String, dynamic> payload;

  Map<String, dynamic> toJson() => {'topic': topic, 'payload': payload};

  factory WsEnvelope.fromJson(Map<String, dynamic> json) => WsEnvelope(
        topic: json['topic'] as String,
        payload: Map<String, dynamic>.from(json['payload'] as Map),
      );
}

enum DryerStatus { idle, drying, stopped, error, unknown }

DryerStatus dryerStatusFromString(String? raw) {
  switch (raw) {
    case 'idle':
      return DryerStatus.idle;
    case 'drying':
      return DryerStatus.drying;
    case 'stopped':
      return DryerStatus.stopped;
    case 'error':
      return DryerStatus.error;
    default:
      return DryerStatus.unknown;
  }
}

class StatusSnapshot {
  const StatusSnapshot({
    required this.status,
    this.chamberTempC,
    this.targetTempC,
    this.humidityPct,
    this.targetHumidityPct,
    this.heaterOn = false,
    this.heaterPowerPct = 0,
    this.exhaustFanOn = false,
    this.exhaustFanPowerPct = 0,
    this.elapsedTimeSec = 0,
    this.remainingTimeSec = 0,
    this.cpuUsagePct,
    this.memoryFreeBytes,
    this.uptimeSec = 0,
  });

  final DryerStatus status;
  final double? chamberTempC;
  final double? targetTempC;
  final double? humidityPct;
  final double? targetHumidityPct;
  final bool heaterOn;
  final double heaterPowerPct;
  final bool exhaustFanOn;
  final double exhaustFanPowerPct;
  final int elapsedTimeSec;
  final int remainingTimeSec;
  final double? cpuUsagePct;
  final int? memoryFreeBytes;
  final int uptimeSec;

  bool get hasSensorError =>
      status == DryerStatus.drying &&
      (chamberTempC == null || chamberTempC!.isNaN);

  factory StatusSnapshot.fromPayload(Map<String, dynamic> payload) {
    return StatusSnapshot(
      status: dryerStatusFromString(payload['status'] as String?),
      chamberTempC: (payload['chamber_temp_c'] as num?)?.toDouble(),
      targetTempC: (payload['target_temp_c'] as num?)?.toDouble(),
      humidityPct: (payload['humidity_pct'] as num?)?.toDouble(),
      targetHumidityPct: (payload['target_humidity_pct'] as num?)?.toDouble(),
      heaterOn: payload['heater_on'] as bool? ?? false,
      heaterPowerPct: (payload['heater_power_pct'] as num?)?.toDouble() ?? 0,
      exhaustFanOn: payload['exhaust_fan_on'] as bool? ?? false,
      exhaustFanPowerPct:
          (payload['exhaust_fan_power_pct'] as num?)?.toDouble() ?? 0,
      elapsedTimeSec: (payload['elapsed_time_sec'] as num?)?.toInt() ?? 0,
      remainingTimeSec: (payload['remaining_time_sec'] as num?)?.toInt() ?? 0,
      cpuUsagePct: (payload['cpu_usage_pct'] as num?)?.toDouble(),
      memoryFreeBytes: (payload['memory_free_bytes'] as num?)?.toInt(),
      uptimeSec: (payload['uptime_sec'] as num?)?.toInt() ?? 0,
    );
  }
}

class FaultEvent {
  const FaultEvent({
    required this.faultCode,
    required this.message,
    this.actionTaken,
    this.timestampSec = 0,
  });

  final String faultCode;
  final String message;
  final String? actionTaken;
  final int timestampSec;

  factory FaultEvent.fromPayload(Map<String, dynamic> payload) => FaultEvent(
        faultCode: payload['fault_code'] as String? ?? 'UNKNOWN',
        message: payload['message'] as String? ?? '',
        actionTaken: payload['action_taken'] as String?,
        timestampSec: (payload['timestamp_sec'] as num?)?.toInt() ?? 0,
      );
}

class StartCycleRequest {
  const StartCycleRequest({
    this.profileId,
    this.targetTempC,
    this.maxDurationMin,
    this.targetHumidityPct,
  });

  final String? profileId;
  final double? targetTempC;
  final int? maxDurationMin;
  final double? targetHumidityPct;

  Map<String, dynamic> toPayload() {
    final map = <String, dynamic>{};
    if (profileId != null) map['profile_id'] = profileId;
    if (targetTempC != null) map['target_temp_c'] = targetTempC;
    if (maxDurationMin != null) map['max_duration_min'] = maxDurationMin;
    if (targetHumidityPct != null) {
      map['target_humidity_pct'] = targetHumidityPct;
    }
    return map;
  }
}

class StopCycleRequest {
  const StopCycleRequest({this.reason = 'user_requested'});

  final String reason;

  Map<String, dynamic> toPayload() => {'reason': reason};
}
