/*
 * Philarmony Filament Dryer — Desktop Installer / Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import 'filament_profile.dart';

/// Device configuration accumulated by the installer wizard.
class DeviceProfile {
  DeviceProfile({
    this.profileVersion = 1,
    this.deviceModel = 'ESP32',
    this.flashSizeMb = 4,
    List<SensorConfig>? sensors,
    PinMapping? pinMapping,
    DisplayConfig? display,
    List<FilamentProfile>? filamentProfiles,
    WiFiConfig? wifi,
    this.firmwareVersion = '0.1.0',
    DateTime? createdAt,
    DateTime? updatedAt,
  })  : sensors = sensors ?? [],
        pinMapping = pinMapping ?? PinMapping(heaterPwm: 25),
        display = display ?? DisplayConfig(),
        filamentProfiles = filamentProfiles ?? FilamentProfile.builtins(),
        wifi = wifi ?? WiFiConfig(),
        createdAt = createdAt ?? DateTime.now().toUtc(),
        updatedAt = updatedAt ?? DateTime.now().toUtc();

  int profileVersion;
  String deviceModel;
  int flashSizeMb;
  List<SensorConfig> sensors;
  PinMapping pinMapping;
  DisplayConfig display;
  List<FilamentProfile> filamentProfiles;
  WiFiConfig wifi;
  String firmwareVersion;
  DateTime createdAt;
  DateTime updatedAt;

  DeviceProfile copyWith({
    String? deviceModel,
    int? flashSizeMb,
    List<SensorConfig>? sensors,
    PinMapping? pinMapping,
    DisplayConfig? display,
    List<FilamentProfile>? filamentProfiles,
    WiFiConfig? wifi,
    String? firmwareVersion,
  }) {
    return DeviceProfile(
      profileVersion: profileVersion,
      deviceModel: deviceModel ?? this.deviceModel,
      flashSizeMb: flashSizeMb ?? this.flashSizeMb,
      sensors: sensors ?? List.of(this.sensors),
      pinMapping: pinMapping ?? this.pinMapping,
      display: display ?? this.display,
      filamentProfiles: filamentProfiles ?? List.of(this.filamentProfiles),
      wifi: wifi ?? this.wifi,
      firmwareVersion: firmwareVersion ?? this.firmwareVersion,
      createdAt: createdAt,
      updatedAt: DateTime.now().toUtc(),
    );
  }
}

class SensorConfig {
  SensorConfig({
    required this.role,
    required this.sensor,
    this.gpioPin,
    this.i2cAddress,
    Map<String, dynamic>? parameters,
  }) : parameters = parameters ?? {};

  String role;
  String sensor;
  int? gpioPin;
  int? i2cAddress;
  Map<String, dynamic> parameters;
}

class PinMapping {
  PinMapping({
    required this.heaterPwm,
    this.exhaustFanPwm,
    this.exhaustFanDigital,
    this.i2cSda,
    this.i2cScl,
    this.spiMosi,
    this.spiMiso,
    this.spiSck,
    this.spiCs,
    this.spiDc,
    this.spiReset,
  });

  int heaterPwm;
  int? exhaustFanPwm;
  int? exhaustFanDigital;
  int? i2cSda;
  int? i2cScl;
  int? spiMosi;
  int? spiMiso;
  int? spiSck;
  int? spiCs;
  int? spiDc;
  int? spiReset;

  Iterable<MapEntry<String, int>> assignedPins() sync* {
    yield MapEntry('heater_pwm', heaterPwm);
    if (exhaustFanPwm != null) yield MapEntry('exhaust_fan_pwm', exhaustFanPwm!);
    if (exhaustFanDigital != null) {
      yield MapEntry('exhaust_fan_digital', exhaustFanDigital!);
    }
    if (i2cSda != null) yield MapEntry('i2c_sda', i2cSda!);
    if (i2cScl != null) yield MapEntry('i2c_scl', i2cScl!);
    if (spiMosi != null) yield MapEntry('spi_mosi', spiMosi!);
    if (spiMiso != null) yield MapEntry('spi_miso', spiMiso!);
    if (spiSck != null) yield MapEntry('spi_sck', spiSck!);
    if (spiCs != null) yield MapEntry('spi_cs', spiCs!);
    if (spiDc != null) yield MapEntry('spi_dc', spiDc!);
    if (spiReset != null) yield MapEntry('spi_reset', spiReset!);
  }
}

class DisplayConfig {
  DisplayConfig({
    this.enabled = false,
    this.driver = 'none',
    this.width = 128,
    this.height = 64,
    this.i2cAddress,
    List<String>? fields,
    this.refreshRateHz = 1,
  }) : fields = fields ??
            ['chamber_temp_c', 'humidity_pct', 'heater_power_pct', 'status'];

  bool enabled;
  String driver;
  int width;
  int height;
  int? i2cAddress;
  List<String> fields;
  int refreshRateHz;
}

class WiFiConfig {
  WiFiConfig({
    this.ssid = '',
    this.password = '',
    this.staticIp,
  });

  String ssid;
  String password;
  Map<String, String>? staticIp;

  bool get hasPassword => password.isNotEmpty && password != '***';
}
