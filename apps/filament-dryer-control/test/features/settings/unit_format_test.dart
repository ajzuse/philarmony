import 'package:filament_dryer_control/data/app_preferences.dart';
import 'package:filament_dryer_control/device/last_valid_readings.dart';
import 'package:filament_dryer_control/features/settings/unit_format.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';

void main() {
  test('formatHumidity uses %RH suffix', () {
    expect(formatHumidity(12.3), '12.3 %RH');
    expect(formatHumidity(null), '—');
  });

  test('formatDuration respects 12h and 24h formats', () {
    expect(
      formatDuration(3661, timeFormat: AppTimeFormat.h24),
      '01:01:01',
    );
    expect(
      formatDuration(3661, timeFormat: AppTimeFormat.h12),
      '1:01:01 AM',
    );
  });

  test('LastValidReadings keeps last valid chamber temp and humidity', () {
    final readings = LastValidReadings();
    readings.update(const StatusSnapshot(
      status: DryerStatus.drying,
      chamberTempC: 45,
      humidityPct: 20,
    ));
    readings.update(const StatusSnapshot(
      status: DryerStatus.drying,
      chamberTempC: double.nan,
      humidityPct: null,
    ));

    expect(readings.chamberTempC, 45);
    expect(readings.humidityPct, 20);
  });
}
