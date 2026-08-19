import 'package:philarmony_core/philarmony_core.dart';
import 'package:test/test.dart';

void main() {
  test('encode/decode envelope', () {
    final env = WsCodec.startCycle(
      const StartCycleRequest(
        profileId: 'pla',
        targetTempC: 50,
        maxDurationMin: 120,
      ),
    );
    final decoded = WsCodec.decode(WsCodec.encode(env))!;
    expect(decoded.topic, 'control/start');
    expect(decoded.payload['profile_id'], 'pla');
    expect(decoded.payload['max_duration_min'], 120);
  });

  test('parse status update', () {
    final snap = WsCodec.parseStatusUpdate(
      WsEnvelope.fromJson({
        'topic': 'status/update',
        'payload': {
          'status': 'drying',
          'chamber_temp_c': 48.5,
          'target_temp_c': 50,
          'humidity_pct': 22.1,
          'heater_on': true,
          'heater_power_pct': 65,
        },
      }),
    );
    expect(snap!.status, DryerStatus.drying);
    expect(snap.chamberTempC, 48.5);
  });

  test('parse hardware config response and error', () {
    final response = WsCodec.parseHardwareConfigResponse(
      WsEnvelope.fromJson({
        'topic': 'config/hardware/response',
        'payload': {'status': 'saved'},
      }),
    );
    expect(response!.status, 'saved');

    final error = WsCodec.parseHardwareConfigError(
      WsEnvelope.fromJson({
        'topic': 'config/hardware/error',
        'payload': {'error': 'Pin conflict: GPIO 25'},
      }),
    );
    expect(error!.error, contains('Pin conflict'));
  });
}
