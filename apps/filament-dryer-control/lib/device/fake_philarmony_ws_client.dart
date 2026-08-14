/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';

import 'package:philarmony_core/philarmony_core.dart';

import 'device_interfaces.dart';

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
  Timer? _timer;
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
  Future<void> connect(KnownDevice device) async {
    _setState(DeviceConnectionState.connecting);
    await Future<void>.delayed(const Duration(milliseconds: 50));
    _lastHardwareConfig ??= HardwareConfig.defaults();
    _setState(DeviceConnectionState.connected);
    await subscribeStatus();
  }

  @override
  Future<void> disconnect() async {
    _timer?.cancel();
    _setState(DeviceConnectionState.disconnected);
  }

  @override
  Future<void> send(WsEnvelope envelope) async {
    if (envelope.topic.startsWith('config/profiles/')) {
      await request(envelope);
      return;
    }
    if (envelope.topic == 'control/start') {
      _emitStatus(const StatusSnapshot(status: DryerStatus.drying, chamberTempC: 45));
    } else if (envelope.topic == 'control/stop') {
      _emitStatus(const StatusSnapshot(status: DryerStatus.stopped, chamberTempC: 40));
    }
  }

  @override
  Future<WsEnvelope?> request(
    WsEnvelope envelope, {
    Duration timeout = const Duration(seconds: 5),
  }) async {
    switch (envelope.topic) {
      case 'config/profiles/list':
        return _profilesListResponse();
      case 'config/profiles/create':
        return _profilesCreate(envelope.payload);
      case 'config/profiles/update':
        return _profilesUpdate(envelope.payload);
      case 'config/profiles/delete':
        return _profilesDelete(envelope.payload);
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
      ),
    );
    _timer = Timer.periodic(const Duration(seconds: 1), (_) {
      final base = _lastStatus ?? const StatusSnapshot(status: DryerStatus.idle);
      _emitStatus(
        StatusSnapshot(
          status: base.status,
          chamberTempC: (base.chamberTempC ?? 25) + 0.1,
          targetTempC: base.targetTempC,
          humidityPct: base.humidityPct,
          heaterOn: base.status == DryerStatus.drying,
          heaterPowerPct: base.status == DryerStatus.drying ? 55 : 0,
          elapsedTimeSec: base.elapsedTimeSec + 1,
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
    if (!_stateCtrl.isClosed) _stateCtrl.close();
    if (!_statusCtrl.isClosed) _statusCtrl.close();
    if (!_faultCtrl.isClosed) _faultCtrl.close();
    if (!_envelopeCtrl.isClosed) _envelopeCtrl.close();
    if (!_hwResponseCtrl.isClosed) _hwResponseCtrl.close();
    if (!_hwErrorCtrl.isClosed) _hwErrorCtrl.close();
  }
}
