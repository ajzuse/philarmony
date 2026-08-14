/*
 * Philarmony Filament Dryer — Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:convert';

import '../models/hardware_config.dart';
import '../models/ws_models.dart';

/// JSON topic framing per `001` websocket-api contract.
class WsCodec {
  static String encode(WsEnvelope envelope) => jsonEncode(envelope.toJson());

  static WsEnvelope? decode(String raw) {
    try {
      final map = jsonDecode(raw) as Map<String, dynamic>;
      return WsEnvelope.fromJson(map);
    } catch (_) {
      return null;
    }
  }

  static WsEnvelope subscribeStatus() =>
      const WsEnvelope(topic: 'status/subscribe', payload: {});

  static WsEnvelope startCycle(StartCycleRequest request) => WsEnvelope(
        topic: 'control/start',
        payload: request.toPayload(),
      );

  static WsEnvelope stopCycle([StopCycleRequest request = const StopCycleRequest()]) =>
      WsEnvelope(topic: 'control/stop', payload: request.toPayload());

  static WsEnvelope hardwareConfig(Map<String, dynamic> hardware) => WsEnvelope(
        topic: 'config/hardware',
        payload: hardware,
      );

  static StatusSnapshot? parseStatusUpdate(WsEnvelope envelope) {
    if (envelope.topic != 'status/update') return null;
    return StatusSnapshot.fromPayload(envelope.payload);
  }

  static FaultEvent? parseFault(WsEnvelope envelope) {
    if (envelope.topic != 'status/fault') return null;
    return FaultEvent.fromPayload(envelope.payload);
  }

  static HardwareConfigResponse? parseHardwareConfigResponse(WsEnvelope envelope) {
    if (envelope.topic != 'config/hardware/response') return null;
    return HardwareConfigResponse.fromPayload(envelope.payload);
  }

  static HardwareConfigError? parseHardwareConfigError(WsEnvelope envelope) {
    if (envelope.topic != 'config/hardware/error') return null;
    return HardwareConfigError.fromPayload(envelope.payload);
  }
}
