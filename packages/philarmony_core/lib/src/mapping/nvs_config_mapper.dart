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

/// Maps installer DeviceProfile → firmware hardware JSON / NVS-oriented payload.
class NvsConfigMapper {
  Map<String, dynamic> toHardwareJson(DeviceProfile profile) {
    final sensors = <Map<String, dynamic>>[];
    for (final s in profile.sensors) {
      if (s.sensor == 'none') continue;
      sensors.add({
        'role': s.role,
        'type': s.sensor,
        'gpio': s.gpioPin,
        if (s.i2cAddress != null) 'i2c_address': s.i2cAddress,
        'parameters': s.parameters,
      });
    }

    final actuators = <Map<String, dynamic>>[
      {
        'id': 'heater',
        'type': 'mosfet_pwm',
        'gpio': profile.pinMapping.heaterPwm,
      },
    ];
    if (profile.pinMapping.exhaustFanPwm != null) {
      actuators.add({
        'id': 'exhaust_fan',
        'type': 'fan_pwm',
        'gpio': profile.pinMapping.exhaustFanPwm,
      });
    } else if (profile.pinMapping.exhaustFanDigital != null) {
      actuators.add({
        'id': 'exhaust_fan',
        'type': 'fan_digital',
        'gpio': profile.pinMapping.exhaustFanDigital,
      });
    }

    final display = <String, dynamic>{
      'enabled': profile.display.enabled,
      'driver': profile.display.driver,
      'width': profile.display.width,
      'height': profile.display.height,
      'fields': profile.display.fields,
      'refresh_rate_hz': profile.display.refreshRateHz,
    };
    if (profile.display.i2cAddress != null) {
      display['i2c_address'] = profile.display.i2cAddress;
    }
    if (profile.pinMapping.i2cSda != null) {
      display['i2c_sda'] = profile.pinMapping.i2cSda;
      display['i2c_scl'] = profile.pinMapping.i2cScl;
    }

    return {
      'sensors': sensors,
      'actuators': actuators,
      'display': display,
      'control': {
        'algorithm': 'pid',
        'safety_limits': {'max_temp_c': 80},
      },
      'bus': {
        if (profile.pinMapping.i2cSda != null) 'i2c_sda': profile.pinMapping.i2cSda,
        if (profile.pinMapping.i2cScl != null) 'i2c_scl': profile.pinMapping.i2cScl,
        if (profile.pinMapping.spiMosi != null) 'spi_mosi': profile.pinMapping.spiMosi,
        if (profile.pinMapping.spiMiso != null) 'spi_miso': profile.pinMapping.spiMiso,
        if (profile.pinMapping.spiSck != null) 'spi_sck': profile.pinMapping.spiSck,
        if (profile.pinMapping.spiCs != null) 'spi_cs': profile.pinMapping.spiCs,
      },
    };
  }

  Map<String, dynamic> toWifiNvs(DeviceProfile profile) => {
        'ssid': profile.wifi.ssid,
        'password': profile.wifi.password,
        if (profile.wifi.staticIp != null) 'static_ip': profile.wifi.staticIp,
      };

  List<Map<String, dynamic>> toFilamentProfilesNvs(DeviceProfile profile) =>
      profile.filamentProfiles
          .map((f) => {
                'id': f.id,
                'name_pt': f.namePt,
                'name_en': f.nameEn,
                'target_temp_c': f.targetTempC,
                'default_duration_min': f.defaultDurationMin,
                'target_humidity_pct': f.targetHumidityPct,
                'is_builtin': f.isBuiltin,
              })
          .toList();

  /// Combined flash-time config blob (API / sidecar debug).
  Map<String, dynamic> toFlashConfigBlob(DeviceProfile profile) => {
        'hardware': toHardwareJson(profile),
        'wifi': toWifiNvs(profile),
        'filament_profiles': toFilamentProfilesNvs(profile),
        'firmware_version': profile.firmwareVersion,
      };

  /// Flat Preferences JSON shapes for namespace `filament_dryer` (boot path).
  Map<String, String> toPreferencesStringEntries(DeviceProfile profile) {
    SensorConfig? temp;
    SensorConfig? hum;
    for (final s in profile.sensors) {
      if (s.sensor == 'none') continue;
      final role = s.role.toLowerCase();
      if (role.contains('humid')) {
        hum ??= s;
      } else {
        temp ??= s;
      }
    }
    if (temp == null) {
      for (final s in profile.sensors) {
        if (s.sensor != 'none') {
          temp = s;
          break;
        }
      }
    }

    final sda = profile.pinMapping.i2cSda ?? 21;
    final scl = profile.pinMapping.i2cScl ?? 22;
    final integrated = hum == null ||
        (temp != null &&
            hum.sensor == temp.sensor &&
            hum.gpioPin == temp.gpioPin);

    final sensorType =
        temp == null || temp.sensor == 'none' ? 'sht31' : temp.sensor;

    final sensor = <String, dynamic>{
      'type': sensorType,
      'is_integrated': integrated,
      'i2c_bus': 0,
      'i2c_address': temp?.i2cAddress ?? 0x44,
      'gpio_pin': temp?.gpioPin ?? -1,
      'sda_pin': sda,
      'scl_pin': scl,
      'temperature_offset': 0.0,
      'temperature_scale': 1.0,
      'humidity_offset': 0.0,
      'humidity_scale': 1.0,
      'humidity_type': integrated ? '' : (hum?.sensor ?? ''),
      'humidity_i2c_address': integrated ? 0 : (hum?.i2cAddress ?? 0),
      'humidity_gpio_pin': integrated ? -1 : (hum?.gpioPin ?? -1),
      'humidity_sda_pin': sda,
      'humidity_scl_pin': scl,
      'extra_temp_type': '',
      'extra_temp_gpio_pin': -1,
      'extra_temp_i2c_address': 0,
    };

    final fanIsPwm = profile.pinMapping.exhaustFanPwm != null;
    final actuator = <String, dynamic>{
      'heater_type': 'mosfet_pwm',
      'heater_pin': profile.pinMapping.heaterPwm,
      'heater_pwm_freq': 1000,
      'heater_max_power_pct': 100,
      'heater_max_temp_c': 80.0,
      'fan_type': fanIsPwm ? 'fan_pwm' : 'fan_digital',
      'fan_mode': 'independent_pwm',
      'fan_pin': profile.pinMapping.exhaustFanPwm ??
          profile.pinMapping.exhaustFanDigital ??
          26,
      'fan_pwm_freq': 5000,
      'fan_duty_pct': 80.0,
      'cooldown_duration_sec': 30,
      'has_custom': false,
      'custom_type': '',
      'custom_pin': -1,
      'fan_speed_curve': <Map<String, dynamic>>[],
    };

    final display = <String, dynamic>{
      'enabled': profile.display.enabled,
      'driver': profile.display.driver,
      'bus_type': profile.pinMapping.spiMosi != null ? 'spi' : 'i2c',
      'width': profile.display.width,
      'height': profile.display.height,
      'rotation': 0,
      'spi_mosi': profile.pinMapping.spiMosi ?? -1,
      'spi_sclk': profile.pinMapping.spiSck ?? -1,
      'spi_cs': profile.pinMapping.spiCs ?? -1,
      'dc_pin': profile.pinMapping.spiDc ?? -1,
      'rst_pin': profile.pinMapping.spiReset ?? -1,
      'backlight_pin': -1,
      'i2c_sda': sda,
      'i2c_scl': scl,
      'i2c_address': profile.display.i2cAddress ?? 0x3C,
      'refresh_rate_hz': profile.display.refreshRateHz,
      'font_scaling': 'auto',
      'compact_mode': false,
      'fields': profile.display.fields,
    };

    final control = <String, dynamic>{
      'algorithm': 'pid',
      'auto_tune': false,
      'safety_limits': {
        'hard_temp_limit_c': 80.0,
        'max_heater_power_pct': 100,
        'sensor_timeout_ms': 600,
        'thermal_runaway_time_sec': 45,
        'thermal_runaway_temp_rise_c': 0.5,
      },
    };

    String enc(Object o) => jsonEncode(o);

    return {
      'wifi': enc(toWifiNvs(profile)),
      'sensor': enc(sensor),
      'actuator': enc(actuator),
      'display': enc(display),
      'profiles': enc(toFilamentProfilesNvs(profile)),
      'objects': enc(['control']),
      'obj_control': enc(control),
    };
  }
}
