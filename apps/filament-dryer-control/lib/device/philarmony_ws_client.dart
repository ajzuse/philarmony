/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

import 'device_interfaces.dart';

class PhilarmonyWsClientImpl implements PhilarmonyWsClient {
  PhilarmonyWsClientImpl();

  final _stateCtrl = StreamController<DeviceConnectionState>.broadcast();
  final _statusCtrl = StreamController<StatusSnapshot>.broadcast();
  final _faultCtrl = StreamController<FaultEvent>.broadcast();
  final _envelopeCtrl = StreamController<WsEnvelope>.broadcast();
  final _hwResponseCtrl = StreamController<HardwareConfigResponse>.broadcast();
  final _hwErrorCtrl = StreamController<HardwareConfigError>.broadcast();

  WebSocketChannel? _channel;
  StreamSubscription<dynamic>? _sub;
  DeviceConnectionState _state = DeviceConnectionState.idle;
  StatusSnapshot? _lastStatus;
  HardwareConfig? _lastHardwareConfig;
  KnownDevice? _device;
  int _reconnectAttempt = 0;
  Timer? _reconnectTimer;

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
    _device = device;
    await _openSocket();
  }

  Future<void> _openSocket() async {
    final device = _device;
    if (device == null) return;
    _setState(DeviceConnectionState.connecting);
    try {
      await _sub?.cancel();
      await _channel?.sink.close();
      _channel = WebSocketChannel.connect(Uri.parse(device.wsUri));
      _sub = _channel!.stream.listen(
        _onMessage,
        onError: (_) => _scheduleReconnect(),
        onDone: () => _scheduleReconnect(),
      );
      _reconnectAttempt = 0;
      _setState(DeviceConnectionState.connected);
      await subscribeStatus();
    } catch (_) {
      _setState(DeviceConnectionState.error);
      _scheduleReconnect();
    }
  }

  void _onMessage(dynamic raw) {
    final envelope = WsCodec.decode(raw as String);
    if (envelope == null) return;
    _envelopeCtrl.add(envelope);
    final status = WsCodec.parseStatusUpdate(envelope);
    if (status != null) {
      _lastStatus = status;
      _statusCtrl.add(status);
    }
    final fault = WsCodec.parseFault(envelope);
    if (fault != null) _faultCtrl.add(fault);
    final hwResponse = WsCodec.parseHardwareConfigResponse(envelope);
    if (hwResponse != null) _hwResponseCtrl.add(hwResponse);
    final hwError = WsCodec.parseHardwareConfigError(envelope);
    if (hwError != null) _hwErrorCtrl.add(hwError);
  }

  void _scheduleReconnect() {
    if (_device == null) return;
    _setState(DeviceConnectionState.reconnecting);
    _reconnectTimer?.cancel();
    final delay = Duration(
      seconds: (1 << _reconnectAttempt).clamp(1, 60),
    );
    _reconnectAttempt++;
    _reconnectTimer = Timer(delay, _openSocket);
  }

  @override
  Future<void> disconnect() async {
    _reconnectTimer?.cancel();
    _device = null;
    await _sub?.cancel();
    await _channel?.sink.close();
    _setState(DeviceConnectionState.disconnected);
  }

  @override
  Future<void> send(WsEnvelope envelope) async {
    final ch = _channel;
    if (ch == null) throw StateError('not connected');
    ch.sink.add(WsCodec.encode(envelope));
  }

  @override
  Future<WsEnvelope?> request(
    WsEnvelope envelope, {
    Duration timeout = const Duration(seconds: 5),
  }) async {
    final responseTopic = '${envelope.topic}/response';
    final completer = Completer<WsEnvelope?>();
    late final StreamSubscription<WsEnvelope> sub;
    sub = _envelopeCtrl.stream.listen((msg) {
      if (msg.topic == responseTopic || msg.topic == '${envelope.topic}/error') {
        if (!completer.isCompleted) completer.complete(msg);
        sub.cancel();
      }
    });
    await send(envelope);
    try {
      return await completer.future.timeout(timeout);
    } on TimeoutException {
      await sub.cancel();
      throw TimeoutException('No response for ${envelope.topic}');
    }
  }

  @override
  Future<void> subscribeStatus() => send(WsCodec.subscribeStatus());

  @override
  Future<void> startCycle(StartCycleRequest request) async {
    final errors = CycleCommandValidator.validateStart(request);
    if (errors.isNotEmpty) throw ArgumentError(errors.join('; '));
    await send(WsCodec.startCycle(request));
  }

  @override
  Future<void> stopCycle([StopCycleRequest request = const StopCycleRequest()]) =>
      send(WsCodec.stopCycle(request));

  @override
  Future<void> sendHardwareConfig(HardwareConfig config) async {
    _lastHardwareConfig = config;
    await send(WsCodec.hardwareConfig(config.toPayload()));
  }

  void _setState(DeviceConnectionState next) {
    _state = next;
    _stateCtrl.add(next);
  }

  void dispose() {
    _reconnectTimer?.cancel();
    _sub?.cancel();
    _channel?.sink.close();
    _stateCtrl.close();
    _statusCtrl.close();
    _faultCtrl.close();
    _envelopeCtrl.close();
    _hwResponseCtrl.close();
    _hwErrorCtrl.close();
  }
}
