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

import 'dart:convert';

import '../models/device_profile.dart';
import '../models/filament_profile.dart';

/// JSON codec for installer profiles (schema v1).
class ProfileCodec {
  static const redactedPassword = '***';

  Map<String, dynamic> toJson(
    DeviceProfile profile, {
    bool redactPassword = true,
  }) {
    return {
      'profile_version': profile.profileVersion,
      'device_model': profile.deviceModel,
      'flash_size_mb': profile.flashSizeMb,
      'sensors': profile.sensors
          .map((s) => {
                'role': s.role,
                'sensor': s.sensor,
                'gpio_pin': s.gpioPin,
                'i2c_address': s.i2cAddress,
                'parameters': s.parameters,
              })
          .toList(),
      'pin_mapping': {
        'heater_pwm': profile.pinMapping.heaterPwm,
        'exhaust_fan_pwm': profile.pinMapping.exhaustFanPwm,
        'exhaust_fan_digital': profile.pinMapping.exhaustFanDigital,
        'i2c_sda': profile.pinMapping.i2cSda,
        'i2c_scl': profile.pinMapping.i2cScl,
        'spi_mosi': profile.pinMapping.spiMosi,
        'spi_miso': profile.pinMapping.spiMiso,
        'spi_sck': profile.pinMapping.spiSck,
        'spi_cs': profile.pinMapping.spiCs,
        'spi_dc': profile.pinMapping.spiDc,
        'spi_reset': profile.pinMapping.spiReset,
      },
      'display': {
        'enabled': profile.display.enabled,
        'driver': profile.display.driver,
        'width': profile.display.width,
        'height': profile.display.height,
        'i2c_address': profile.display.i2cAddress,
        'fields': profile.display.fields,
        'refresh_rate_hz': profile.display.refreshRateHz,
      },
      'filament_profiles': profile.filamentProfiles
          .map((f) => {
                'id': f.id,
                'name_pt': f.namePt,
                'name_en': f.nameEn,
                'target_temp_c': f.targetTempC,
                'default_duration_min': f.defaultDurationMin,
                'target_humidity_pct': f.targetHumidityPct,
                'is_builtin': f.isBuiltin,
              })
          .toList(),
      'wifi': {
        'ssid': profile.wifi.ssid,
        'password': redactPassword ? redactedPassword : profile.wifi.password,
        'static_ip': profile.wifi.staticIp,
      },
      'firmware_version': profile.firmwareVersion,
      'created_at': profile.createdAt.toIso8601String(),
      'updated_at': profile.updatedAt.toIso8601String(),
    };
  }

  String encode(DeviceProfile profile, {bool redactPassword = true}) =>
      const JsonEncoder.withIndent('  ').convert(
        toJson(profile, redactPassword: redactPassword),
      );

  DeviceProfile decode(String source) {
    late final Map<String, dynamic> map;
    try {
      map = jsonDecode(source) as Map<String, dynamic>;
    } catch (e) {
      throw FormatException('Invalid JSON: $e');
    }
    _validateSchema(map);

    final pins = map['pin_mapping'] as Map<String, dynamic>? ?? {};
    final display = map['display'] as Map<String, dynamic>? ?? {};
    final wifi = map['wifi'] as Map<String, dynamic>? ?? {};

    return DeviceProfile(
      profileVersion: 1,
      deviceModel: map['device_model'] as String? ?? 'ESP32',
      flashSizeMb: map['flash_size_mb'] as int? ?? 4,
      sensors: (map['sensors'] as List<dynamic>? ?? [])
          .map((e) {
            final m = e as Map<String, dynamic>;
            return SensorConfig(
              role: m['role'] as String,
              sensor: m['sensor'] as String,
              gpioPin: m['gpio_pin'] as int?,
              i2cAddress: m['i2c_address'] as int?,
              parameters: Map<String, dynamic>.from(
                m['parameters'] as Map? ?? {},
              ),
            );
          })
          .toList(),
      pinMapping: PinMapping(
        heaterPwm: pins['heater_pwm'] as int? ?? 25,
        exhaustFanPwm: pins['exhaust_fan_pwm'] as int?,
        exhaustFanDigital: pins['exhaust_fan_digital'] as int?,
        i2cSda: pins['i2c_sda'] as int?,
        i2cScl: pins['i2c_scl'] as int?,
        spiMosi: pins['spi_mosi'] as int?,
        spiMiso: pins['spi_miso'] as int?,
        spiSck: pins['spi_sck'] as int?,
        spiCs: pins['spi_cs'] as int?,
        spiDc: pins['spi_dc'] as int?,
        spiReset: pins['spi_reset'] as int?,
      ),
      display: DisplayConfig(
        enabled: display['enabled'] as bool? ?? false,
        driver: display['driver'] as String? ?? 'none',
        width: display['width'] as int? ?? 128,
        height: display['height'] as int? ?? 64,
        i2cAddress: display['i2c_address'] as int?,
        fields: List<String>.from(display['fields'] as List? ?? const []),
        refreshRateHz: display['refresh_rate_hz'] as int? ?? 1,
      ),
      filamentProfiles: (map['filament_profiles'] as List<dynamic>? ?? [])
          .map((e) {
            final m = e as Map<String, dynamic>;
            return FilamentProfile(
              id: m['id'] as String,
              namePt: m['name_pt'] as String? ?? '',
              nameEn: m['name_en'] as String? ?? '',
              targetTempC: (m['target_temp_c'] as num?)?.toDouble() ?? 50,
              defaultDurationMin: m['default_duration_min'] as int? ?? 240,
              targetHumidityPct:
                  (m['target_humidity_pct'] as num?)?.toDouble() ?? 15,
              isBuiltin: m['is_builtin'] as bool? ?? false,
            );
          })
          .toList(),
      wifi: WiFiConfig(
        ssid: wifi['ssid'] as String? ?? '',
        password: wifi['password'] as String? ?? '',
        staticIp: wifi['static_ip'] == null
            ? null
            : Map<String, String>.from(
                (wifi['static_ip'] as Map).map(
                  (k, v) => MapEntry(k.toString(), v.toString()),
                ),
              ),
      ),
      firmwareVersion: map['firmware_version'] as String? ?? '0.1.0',
    );
  }

  /// Lightweight validation aligned with `installer-profile.schema.json`.
  void _validateSchema(Map<String, dynamic> map) {
    const required = [
      'profile_version',
      'device_model',
      'flash_size_mb',
      'sensors',
      'pin_mapping',
      'display',
      'filament_profiles',
      'wifi',
      'firmware_version',
    ];
    for (final k in required) {
      if (!map.containsKey(k)) {
        throw FormatException('Missing required field: $k');
      }
    }
    if (map['profile_version'] != 1) {
      throw FormatException(
        'Unsupported profile_version: ${map['profile_version']}',
      );
    }
    const models = {'ESP32', 'ESP32-S2', 'ESP32-S3', 'ESP32-C3'};
    if (!models.contains(map['device_model'])) {
      throw FormatException('Invalid device_model: ${map['device_model']}');
    }
    if (![4, 8, 16].contains(map['flash_size_mb'])) {
      throw FormatException('Invalid flash_size_mb: ${map['flash_size_mb']}');
    }
    if (map['sensors'] is! List) {
      throw FormatException('sensors must be an array');
    }
    if (map['pin_mapping'] is! Map ||
        (map['pin_mapping'] as Map)['heater_pwm'] == null) {
      throw FormatException('pin_mapping.heater_pwm is required');
    }
    if (map['display'] is! Map) {
      throw FormatException('display must be an object');
    }
    if (map['wifi'] is! Map) {
      throw FormatException('wifi must be an object');
    }
  }
}
