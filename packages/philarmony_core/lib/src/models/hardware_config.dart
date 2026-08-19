/*
 * Philarmony Filament Dryer — Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

/// Lightweight hardware config DTO aligned with firmware `config/hardware`.
class HardwareConfig {
  HardwareConfig({
    List<Map<String, dynamic>>? sensors,
    List<Map<String, dynamic>>? actuators,
    Map<String, dynamic>? display,
    Map<String, dynamic>? control,
  })  : sensors = sensors ?? [],
        actuators = actuators ?? [],
        display = display ?? {},
        control = control ?? {};

  List<Map<String, dynamic>> sensors;
  List<Map<String, dynamic>> actuators;
  Map<String, dynamic> display;
  Map<String, dynamic> control;

  Map<String, dynamic> toPayload() => {
        'sensors': sensors,
        'actuators': actuators,
        'display': display,
        'control': control,
      };

  factory HardwareConfig.fromPayload(Map<String, dynamic> p) {
    return HardwareConfig(
      sensors: (p['sensors'] as List?)
              ?.map((e) => (e as Map).cast<String, dynamic>())
              .toList() ??
          [],
      actuators: (p['actuators'] as List?)
              ?.map((e) => (e as Map).cast<String, dynamic>())
              .toList() ??
          [],
      display: (p['display'] as Map?)?.cast<String, dynamic>() ?? {},
      control: (p['control'] as Map?)?.cast<String, dynamic>() ?? {},
    );
  }

  HardwareConfig copyWith({
    List<Map<String, dynamic>>? sensors,
    List<Map<String, dynamic>>? actuators,
    Map<String, dynamic>? display,
    Map<String, dynamic>? control,
  }) {
    return HardwareConfig(
      sensors: sensors ?? List<Map<String, dynamic>>.from(this.sensors),
      actuators: actuators ?? List<Map<String, dynamic>>.from(this.actuators),
      display: display ?? Map<String, dynamic>.from(this.display),
      control: control ?? Map<String, dynamic>.from(this.control),
    );
  }

  /// Starter config aligned with websocket-api.md examples.
  factory HardwareConfig.defaults() => HardwareConfig(
        sensors: [
          {
            'id': 'chamber_temp',
            'type': 'sht3x',
            'capabilities': ['temperature', 'humidity'],
            'bus': {
              'type': 'i2c',
              'bus': 0,
              'address': 68,
              'sda_pin': 21,
              'scl_pin': 22,
            },
          },
        ],
        actuators: [
          {
            'id': 'heater',
            'type': 'mosfet_pwm',
            'role': 'heater',
            'pins': {'pwm': 25},
            'control': {'algorithm': 'pid', 'pwm_freq_hz': 1000, 'max_power_pct': 100},
          },
          {
            'id': 'exhaust_fan',
            'type': 'fan_pwm',
            'role': 'fan',
            'pins': {'pwm': 26},
            'control': {'pwm_freq_hz': 5000, 'cooldown_sec': 30},
          },
        ],
        display: {
          'enabled': true,
          'driver': 'st7789',
          'bus': {
            'type': 'spi',
            'mosi': 19,
            'sclk': 18,
            'cs': 5,
            'dc': 27,
            'rst': 23,
            'bl': 4,
          },
          'geometry': {'width': 135, 'height': 240, 'rotation': 90},
        },
        control: {
          'algorithm': 'pid',
          'parameters': {'kp': 12.5, 'ki': 0.45, 'kd': 32.1},
          'auto_tune': false,
          'safety_limits': {'hard_temp_limit_c': 80, 'sensor_timeout_ms': 600},
        },
      );
}

class HardwareConfigResponse {
  const HardwareConfigResponse({required this.status});

  final String status;

  factory HardwareConfigResponse.fromPayload(Map<String, dynamic> payload) {
    return HardwareConfigResponse(status: payload['status'] as String? ?? 'unknown');
  }
}

class HardwareConfigError {
  const HardwareConfigError({required this.error});

  final String error;

  factory HardwareConfigError.fromPayload(Map<String, dynamic> payload) {
    return HardwareConfigError(
      error: payload['error'] as String? ?? payload['message'] as String? ?? 'Unknown error',
    );
  }
}
