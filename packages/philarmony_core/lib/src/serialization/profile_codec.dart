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
import 'installer_profile_schema.dart';

/// JSON codec for installer profiles (schema v1).
class ProfileCodec {
  static const redactedPassword = '***';

  /// Parsed installer-profile.schema.json (loaded once).
  static final Map<String, dynamic> schema =
      jsonDecode(kInstallerProfileSchemaJson) as Map<String, dynamic>;

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

  /// Validates [map] against embedded `installer-profile.schema.json`.
  void _validateSchema(Map<String, dynamic> map) {
    final required = List<String>.from(schema['required'] as List);
    for (final k in required) {
      if (!map.containsKey(k)) {
        throw FormatException('Missing required field: $k');
      }
    }
    if (schema['additionalProperties'] == false) {
      final allowed = (schema['properties'] as Map).keys.toSet();
      for (final k in map.keys) {
        if (!allowed.contains(k)) {
          throw FormatException('Unknown field: $k');
        }
      }
    }

    final props = schema['properties'] as Map<String, dynamic>;
    _assertConst(map['profile_version'], props['profile_version'], 'profile_version');
    _assertEnum(map['device_model'], props['device_model'], 'device_model');
    _assertEnum(map['flash_size_mb'], props['flash_size_mb'], 'flash_size_mb');
    _assertStringMin(map['firmware_version'], props['firmware_version'], 'firmware_version');

    final sensors = map['sensors'];
    if (sensors is! List) throw FormatException('sensors must be an array');
    final sensorSchema = (props['sensors'] as Map)['items'] as Map<String, dynamic>;
    for (var i = 0; i < sensors.length; i++) {
      final s = sensors[i];
      if (s is! Map) throw FormatException('sensors[$i] must be an object');
      _validateObject(Map<String, dynamic>.from(s), sensorSchema, 'sensors[$i]');
    }

    final pins = map['pin_mapping'];
    if (pins is! Map) throw FormatException('pin_mapping must be an object');
    _validateObject(
      Map<String, dynamic>.from(pins),
      props['pin_mapping'] as Map<String, dynamic>,
      'pin_mapping',
    );

    final display = map['display'];
    if (display is! Map) throw FormatException('display must be an object');
    _validateObject(
      Map<String, dynamic>.from(display),
      props['display'] as Map<String, dynamic>,
      'display',
    );

    final filaments = map['filament_profiles'];
    if (filaments is! List) {
      throw FormatException('filament_profiles must be an array');
    }
    final filSchema = props['filament_profiles'] as Map<String, dynamic>;
    final maxItems = filSchema['maxItems'] as int?;
    if (maxItems != null && filaments.length > maxItems) {
      throw FormatException('filament_profiles exceeds maxItems $maxItems');
    }
    final filItem = filSchema['items'] as Map<String, dynamic>;
    for (var i = 0; i < filaments.length; i++) {
      final f = filaments[i];
      if (f is! Map) {
        throw FormatException('filament_profiles[$i] must be an object');
      }
      _validateObject(
        Map<String, dynamic>.from(f),
        filItem,
        'filament_profiles[$i]',
      );
    }

    final wifi = map['wifi'];
    if (wifi is! Map) throw FormatException('wifi must be an object');
    _validateObject(
      Map<String, dynamic>.from(wifi),
      props['wifi'] as Map<String, dynamic>,
      'wifi',
    );
  }

  void _validateObject(
    Map<String, dynamic> obj,
    Map<String, dynamic> schemaObj,
    String path,
  ) {
    final required = List<String>.from(schemaObj['required'] as List? ?? const []);
    for (final k in required) {
      if (!obj.containsKey(k) || obj[k] == null) {
        throw FormatException('$path.$k is required');
      }
    }
    if (schemaObj['additionalProperties'] == false) {
      final allowed = ((schemaObj['properties'] as Map?) ?? {}).keys.toSet();
      for (final k in obj.keys) {
        if (!allowed.contains(k)) {
          throw FormatException('$path has unknown field: $k');
        }
      }
    }
    final properties = (schemaObj['properties'] as Map<String, dynamic>?) ?? {};
    for (final entry in obj.entries) {
      final propSchema = properties[entry.key];
      if (propSchema is! Map) continue;
      _assertValue(entry.value, Map<String, dynamic>.from(propSchema), '$path.${entry.key}');
    }
  }

  void _assertValue(dynamic value, Map<String, dynamic> prop, String path) {
    if (value == null) {
      final types = prop['type'];
      if (types is List && types.contains('null')) return;
      if (types == 'null') return;
      // null allowed when not required (already checked)
      return;
    }
    if (prop.containsKey('enum')) {
      _assertEnum(value, prop, path);
    }
    if (prop.containsKey('const')) {
      _assertConst(value, prop, path);
    }
    final type = prop['type'];
    final types = type is List ? type.cast<String>() : [if (type is String) type];
    if (types.contains('integer') && value is! int) {
      if (!(types.contains('number') && value is num)) {
        throw FormatException('$path must be integer');
      }
    }
    if (types.contains('number') && value is! num) {
      throw FormatException('$path must be number');
    }
    if (types.contains('string') && value is! String) {
      throw FormatException('$path must be string');
    }
    if (types.contains('boolean') && value is! bool) {
      throw FormatException('$path must be boolean');
    }
    if (types.contains('array') && value is! List) {
      throw FormatException('$path must be array');
    }
    if (types.contains('object') && value is! Map) {
      throw FormatException('$path must be object');
    }
    if (value is num) {
      final min = prop['minimum'];
      final max = prop['maximum'];
      if (min is num && value < min) {
        throw FormatException('$path must be >= $min');
      }
      if (max is num && value > max) {
        throw FormatException('$path must be <= $max');
      }
    }
    if (value is String) {
      final minLen = prop['minLength'];
      if (minLen is int && value.length < minLen) {
        throw FormatException('$path minLength $minLen');
      }
    }
    if (value is Map && prop['properties'] is Map) {
      _validateObject(Map<String, dynamic>.from(value), prop, path);
    }
  }

  void _assertEnum(dynamic value, Map<String, dynamic> prop, String path) {
    final enumVals = prop['enum'] as List?;
    if (enumVals == null) return;
    if (!enumVals.contains(value)) {
      throw FormatException('Invalid $path: $value (allowed: $enumVals)');
    }
  }

  void _assertConst(dynamic value, Map<String, dynamic> prop, String path) {
    if (prop['const'] != null && value != prop['const']) {
      throw FormatException('$path must be ${prop['const']}');
    }
  }

  void _assertStringMin(dynamic value, Map<String, dynamic> prop, String path) {
    if (value is! String || value.isEmpty) {
      throw FormatException('$path must be a non-empty string');
    }
    final minLen = prop['minLength'];
    if (minLen is int && value.length < minLen) {
      throw FormatException('$path minLength $minLen');
    }
  }
}
