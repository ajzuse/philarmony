/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';

import 'package:philarmony_core/philarmony_core.dart';

import 'device_interfaces.dart';
import 'device_runtime_info.dart';

class FakePhilarmonyWsClient implements PhilarmonyWsClient {
  FakePhilarmonyWsClient();

  final _stateCtrl = StreamController<DeviceConnectionState>.broadcast();
  final _statusCtrl = StreamController<StatusSnapshot>.broadcast();
  final _faultCtrl = StreamController<FaultEvent>.broadcast();
  final _envelopeCtrl = StreamController<WsEnvelope>.broadcast();
  final _hwResponseCtrl = StreamController<HardwareConfigResponse>.broadcast();
  final _hwErrorCtrl = StreamController<HardwareConfigError>.broadcast();

  DeviceConnectionState _state = DeviceConnectionState.idle;
  StatusSnapshot? _lastStatus;
  HardwareConfig? _lastHardwareConfig;
  DeviceRuntimeInfo? _deviceRuntimeInfo;
  DateTime? _hardwareConfigUpdatedAt;
  KnownDevice? _device;
  Timer? _timer;
  Timer? _reconnectTimer;
  int _reconnectAttempt = 0;
  bool _connected = false;
  final List<WsEnvelope> _outboundQueue = [];
  final List<FilamentProfile> _profiles = FilamentProfile.builtins();

  @override
  Stream<DeviceConnectionState> get connectionStates => _stateCtrl.stream;

  @override
  Stream<StatusSnapshot> get statusStream => _statusCtrl.stream;

  @override
  Stream<FaultEvent> get faultStream => _faultCtrl.stream;

  @override
  Stream<HardwareConfigResponse> get hardwareConfigResponseStream =>
      _hwResponseCtrl.stream;

  @override
  Stream<HardwareConfigError> get hardwareConfigErrorStream => _hwErrorCtrl.stream;

  @override
  DeviceConnectionState get state => _state;

  @override
  StatusSnapshot? get lastStatus => _lastStatus;

  @override
  HardwareConfig? get lastHardwareConfig => _lastHardwareConfig;

  @override
  DeviceRuntimeInfo? get deviceRuntimeInfo => _deviceRuntimeInfo;

  @override
  DateTime? get hardwareConfigUpdatedAt => _hardwareConfigUpdatedAt;

  @override
  Future<void> connect(KnownDevice device) async {
    _device = device;
    await _establishConnection();
  }

  Future<void> _establishConnection() async {
    _setState(DeviceConnectionState.connecting);
    await Future<void>.delayed(const Duration(milliseconds: 50));
    _lastHardwareConfig ??= HardwareConfig.defaults();
    _deviceRuntimeInfo = DeviceRuntimeInfo(
      wifiSsid: _device?.host == '192.168.4.1' ? 'philarmony' : 'lab-wifi',
      wifiSignalDbm: _device?.host == '192.168.4.1' ? null : -48,
      apMode: _device?.host == '192.168.4.1',
      firmwareVersion: '0.1.0',
      deviceName: _device?.nickname,
      ntpTimezone: 'UTC',
      deviceModel: 'ESP32',
    );
    _connected = true;
    _reconnectAttempt = 0;
    _setState(DeviceConnectionState.connected);
    await subscribeStatus();
    await _flushOutboundQueue();
  }

  void _scheduleReconnect() {
    if (_device == null) return;
    _connected = false;
    _timer?.cancel();
    _setState(DeviceConnectionState.reconnecting);
    _reconnectTimer?.cancel();
    final delay = Duration(
      seconds: (1 << _reconnectAttempt).clamp(1, 60),
    );
    _reconnectAttempt++;
    _reconnectTimer = Timer(delay, () {
      if (_device != null) {
        unawaited(_establishConnection());
      }
    });
  }

  @override
  Future<void> disconnect() async {
    _reconnectTimer?.cancel();
    _device = null;
    _connected = false;
    _outboundQueue.clear();
    _timer?.cancel();
    _setState(DeviceConnectionState.disconnected);
  }

  @override
  Future<void> send(WsEnvelope envelope) async {
    if (!_connected) {
      _outboundQueue.add(envelope);
      return;
    }
    await _dispatch(envelope);
  }

  Future<void> _dispatch(WsEnvelope envelope) async {
    if (envelope.topic.startsWith('config/profiles/')) {
      await request(envelope);
      return;
    }
    if (envelope.topic == 'control/start') {
      _emitStatus(
        (_lastStatus ?? const StatusSnapshot(status: DryerStatus.idle)).copyWith(
          status: DryerStatus.drying,
          chamberTempC: 45,
          heaterOn: true,
          heaterPowerPct: 55,
          exhaustFanOn: true,
          exhaustFanPowerPct: 40,
        ),
      );
    } else if (envelope.topic == 'control/stop') {
      _emitStatus(
        (_lastStatus ?? const StatusSnapshot(status: DryerStatus.drying)).copyWith(
          status: DryerStatus.stopped,
          chamberTempC: 40,
          heaterOn: false,
          heaterPowerPct: 0,
          exhaustFanOn: false,
          exhaustFanPowerPct: 0,
        ),
      );
    }
  }

  Future<void> _flushOutboundQueue() async {
    if (!_connected) return;
    final pending = List<WsEnvelope>.from(_outboundQueue);
    _outboundQueue.clear();
    for (final envelope in pending) {
      await _dispatch(envelope);
    }
  }

  /// Simulates an unexpected connection drop for tests.
  void simulateConnectionLoss() {
    if (_device == null) return;
    _connected = false;
    _timer?.cancel();
    _scheduleReconnect();
  }

  @override
  Future<WsEnvelope?> request(
    WsEnvelope envelope, {
    Duration timeout = const Duration(seconds: 5),
  }) async {
    if (!_connected) {
      _outboundQueue.add(envelope);
      return null;
    }
    switch (envelope.topic) {
      case 'config/profiles/list':
        return _profilesListResponse();
      case 'config/profiles/create':
        return _profilesCreate(envelope.payload);
      case 'config/profiles/update':
        return _profilesUpdate(envelope.payload);
      case 'config/profiles/delete':
        return _profilesDelete(envelope.payload);
      case 'config/profiles/get':
        return _profilesGet(envelope.payload);
      case 'config/profiles/reset_defaults':
        return _profilesResetDefaults();
      default:
        await send(envelope);
        return null;
    }
  }

  WsEnvelope _profilesListResponse() => WsEnvelope(
        topic: 'config/profiles/list/response',
        payload: {
          'profiles': _profiles.map(_profileToJson).toList(),
        },
      );

  WsEnvelope _profilesCreate(Map<String, dynamic> payload) {
    final id = 'custom-${DateTime.now().millisecondsSinceEpoch}';
    final profile = FilamentProfile(
      id: id,
      namePt: payload['name_pt'] as String? ?? 'Custom',
      nameEn: payload['name_en'] as String? ?? 'Custom',
      targetTempC: (payload['target_temp_c'] as num?)?.toDouble() ?? 55,
      defaultDurationMin: payload['default_duration_min'] as int? ?? 180,
      targetHumidityPct:
          (payload['target_humidity_pct'] as num?)?.toDouble() ?? 15,
      isBuiltin: false,
    );
    _profiles.removeWhere((p) => !p.isBuiltin && p.id == profile.id);
    _profiles.add(profile);
    return WsEnvelope(
      topic: 'config/profiles/create/response',
      payload: {'status': 'created', 'profile_id': id},
    );
  }

  WsEnvelope _profilesUpdate(Map<String, dynamic> payload) {
    final id = payload['profile_id'] as String? ?? '';
    final idx = _profiles.indexWhere((p) => p.id == id);
    if (idx >= 0) {
      final existing = _profiles[idx];
      _profiles[idx] = FilamentProfile(
        id: existing.id,
        namePt: payload['name_pt'] as String? ?? existing.namePt,
        nameEn: payload['name_en'] as String? ?? existing.nameEn,
        targetTempC:
            (payload['target_temp_c'] as num?)?.toDouble() ?? existing.targetTempC,
        defaultDurationMin:
            payload['default_duration_min'] as int? ?? existing.defaultDurationMin,
        targetHumidityPct: (payload['target_humidity_pct'] as num?)?.toDouble() ??
            existing.targetHumidityPct,
        isBuiltin: false,
      );
    }
    return WsEnvelope(
      topic: 'config/profiles/update/response',
      payload: {'status': 'updated', 'profile_id': id},
    );
  }

  WsEnvelope _profilesDelete(Map<String, dynamic> payload) {
    final id = payload['profile_id'] as String? ?? '';
    _profiles.removeWhere((p) => !p.isBuiltin && p.id == id);
    return WsEnvelope(
      topic: 'config/profiles/delete/response',
      payload: {'status': 'deleted', 'profile_id': id},
    );
  }

  WsEnvelope _profilesGet(Map<String, dynamic> payload) {
    final id = payload['profile_id'] as String? ?? '';
    final profile = _profiles.where((p) => p.id == id).firstOrNull;
    if (profile == null) {
      return WsEnvelope(
        topic: 'config/profiles/get/error',
        payload: {'error': 'Profile not found'},
      );
    }
    return WsEnvelope(
      topic: 'config/profiles/get/response',
      payload: {'profile': _profileToJson(profile)},
    );
  }

  WsEnvelope _profilesResetDefaults() {
    _profiles
      ..clear()
      ..addAll(FilamentProfile.builtins());
    return WsEnvelope(
      topic: 'config/profiles/reset_defaults/response',
      payload: {
        'status': 'reset',
        'profiles': _profiles.map(_profileToJson).toList(),
      },
    );
  }

  static Map<String, dynamic> _profileToJson(FilamentProfile profile) => {
        'id': profile.id,
        'name_pt': profile.namePt,
        'name_en': profile.nameEn,
        'target_temp_c': profile.targetTempC,
        'default_duration_min': profile.defaultDurationMin,
        'target_humidity_pct': profile.targetHumidityPct,
        'is_builtin': profile.isBuiltin,
      };

  @override
  Future<void> subscribeStatus() async {
    _timer?.cancel();
    _emitStatus(
      StatusSnapshot(
        status: DryerStatus.idle,
        chamberTempC: 25,
        targetTempC: 50,
        humidityPct: 30,
        targetHumidityPct: 15,
        heaterOn: false,
        heaterPowerPct: 0,
        exhaustFanOn: false,
        exhaustFanPowerPct: 0,
        elapsedTimeSec: 0,
        remainingTimeSec: 3600,
        cpuUsagePct: 12.5,
        memoryFreeBytes: 180000,
        uptimeSec: 120,
      ),
    );
    _timer = Timer.periodic(const Duration(seconds: 1), (_) {
      final base = _lastStatus ?? const StatusSnapshot(status: DryerStatus.idle);
      final drying = base.status == DryerStatus.drying;
      _emitStatus(
        StatusSnapshot(
          status: base.status,
          chamberTempC: (base.chamberTempC ?? 25) + (drying ? 0.1 : 0),
          targetTempC: base.targetTempC ?? 50,
          humidityPct: drying
              ? ((base.humidityPct ?? 30) - 0.05).clamp(5.0, 100.0)
              : base.humidityPct,
          targetHumidityPct: base.targetHumidityPct ?? 15,
          heaterOn: drying,
          heaterPowerPct: drying ? 55 : 0,
          exhaustFanOn: drying,
          exhaustFanPowerPct: drying ? 40 : 0,
          elapsedTimeSec: base.elapsedTimeSec + (drying ? 1 : 0),
          remainingTimeSec:
              drying ? (base.remainingTimeSec - 1).clamp(0, 86400) : base.remainingTimeSec,
          cpuUsagePct: 10 + (base.elapsedTimeSec % 20) * 0.5,
          memoryFreeBytes: 180000 - (base.elapsedTimeSec % 100) * 50,
          uptimeSec: base.uptimeSec + 1,
        ),
      );
    });
  }

  @override
  Future<void> startCycle(StartCycleRequest request) =>
      send(WsCodec.startCycle(request));

  @override
  Future<void> stopCycle([StopCycleRequest request = const StopCycleRequest()]) =>
      send(WsCodec.stopCycle(request));

  @override
  Future<void> sendHardwareConfig(HardwareConfig config) async {
    final validation = PinValidator().validateHardwareConfig(config);
    if (!validation.ok) {
      _hwErrorCtrl.add(HardwareConfigError(error: validation.errors.first));
      return;
    }
    _lastHardwareConfig = config;
    _hardwareConfigUpdatedAt = DateTime.now().toUtc();
    _hwResponseCtrl.add(const HardwareConfigResponse(status: 'saved'));
  }

  void _setState(DeviceConnectionState next) {
    _state = next;
    if (!_stateCtrl.isClosed) {
      _stateCtrl.add(next);
    }
  }

  void _emitStatus(StatusSnapshot snap) {
    _lastStatus = snap;
    if (!_statusCtrl.isClosed) {
      _statusCtrl.add(snap);
    }
  }

  void dispose() {
    _timer?.cancel();
    _timer = null;
    _reconnectTimer?.cancel();
    if (!_stateCtrl.isClosed) _stateCtrl.close();
    if (!_statusCtrl.isClosed) _statusCtrl.close();
    if (!_faultCtrl.isClosed) _faultCtrl.close();
    if (!_envelopeCtrl.isClosed) _envelopeCtrl.close();
    if (!_hwResponseCtrl.isClosed) _hwResponseCtrl.close();
    if (!_hwErrorCtrl.isClosed) _hwErrorCtrl.close();
  }
}

extension on StatusSnapshot {
  StatusSnapshot copyWith({
    DryerStatus? status,
    double? chamberTempC,
    double? targetTempC,
    double? humidityPct,
    double? targetHumidityPct,
    bool? heaterOn,
    double? heaterPowerPct,
    bool? exhaustFanOn,
    double? exhaustFanPowerPct,
    int? elapsedTimeSec,
    int? remainingTimeSec,
    double? cpuUsagePct,
    int? memoryFreeBytes,
    int? uptimeSec,
  }) {
    return StatusSnapshot(
      status: status ?? this.status,
      chamberTempC: chamberTempC ?? this.chamberTempC,
      targetTempC: targetTempC ?? this.targetTempC,
      humidityPct: humidityPct ?? this.humidityPct,
      targetHumidityPct: targetHumidityPct ?? this.targetHumidityPct,
      heaterOn: heaterOn ?? this.heaterOn,
      heaterPowerPct: heaterPowerPct ?? this.heaterPowerPct,
      exhaustFanOn: exhaustFanOn ?? this.exhaustFanOn,
      exhaustFanPowerPct: exhaustFanPowerPct ?? this.exhaustFanPowerPct,
      elapsedTimeSec: elapsedTimeSec ?? this.elapsedTimeSec,
      remainingTimeSec: remainingTimeSec ?? this.remainingTimeSec,
      cpuUsagePct: cpuUsagePct ?? this.cpuUsagePct,
      memoryFreeBytes: memoryFreeBytes ?? this.memoryFreeBytes,
      uptimeSec: uptimeSec ?? this.uptimeSec,
    );
  }
}
