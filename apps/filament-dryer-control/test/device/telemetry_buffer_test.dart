import 'package:filament_dryer_control/device/telemetry_buffer.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';

void main() {
  test('TelemetryBuffer stores heater power and trims to maxSamples', () {
    final buffer = TelemetryBuffer(maxSamples: 3);
    for (var i = 0; i < 5; i++) {
      buffer.add(StatusSnapshot(
        status: DryerStatus.drying,
        chamberTempC: 40.0 + i,
        humidityPct: 10.0 + i,
        heaterPowerPct: i * 10,
      ));
    }

    expect(buffer.temps, [42, 43, 44]);
    expect(buffer.humidity, [12, 13, 14]);
    expect(buffer.heaterPower, [20, 30, 40]);
  });

  test('TelemetryBuffer default maxSamples is 3600', () {
    expect(TelemetryBuffer().maxSamples, 3600);
  });
}
