/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../device/session_providers.dart';

enum ConfigSaveState { idle, saving, saved, validationFailed, deviceError }

class ConfigUiState {
  const ConfigUiState({
    required this.draft,
    this.deviceModel = 'ESP32',
    this.saveState = ConfigSaveState.idle,
    this.validationErrors = const [],
    this.deviceError,
  });

  final HardwareConfig draft;
  final String deviceModel;
  final ConfigSaveState saveState;
  final List<String> validationErrors;
  final String? deviceError;

  ConfigUiState copyWith({
    HardwareConfig? draft,
    String? deviceModel,
    ConfigSaveState? saveState,
    List<String>? validationErrors,
    String? deviceError,
    bool clearDeviceError = false,
  }) {
    return ConfigUiState(
      draft: draft ?? this.draft,
      deviceModel: deviceModel ?? this.deviceModel,
      saveState: saveState ?? this.saveState,
      validationErrors: validationErrors ?? this.validationErrors,
      deviceError: clearDeviceError ? null : (deviceError ?? this.deviceError),
    );
  }
}

class ConfigController extends Notifier<ConfigUiState> {
  StreamSubscription<HardwareConfigResponse>? _responseSub;
  StreamSubscription<HardwareConfigError>? _errorSub;

  @override
  ConfigUiState build() {
    final client = ref.read(wsClientProvider);
    _responseSub = client.hardwareConfigResponseStream.listen((_) {
      state = state.copyWith(
        saveState: ConfigSaveState.saved,
        validationErrors: const [],
        clearDeviceError: true,
      );
    });
    _errorSub = client.hardwareConfigErrorStream.listen((err) {
      state = state.copyWith(
        saveState: ConfigSaveState.deviceError,
        deviceError: err.error,
      );
    });
    ref.onDispose(() {
      _responseSub?.cancel();
      _errorSub?.cancel();
    });

    final cached = client.lastHardwareConfig ?? HardwareConfig.defaults();
    return ConfigUiState(draft: cached.copyWith());
  }

  void updateSensorType(int index, String type) {
    final sensors = List<Map<String, dynamic>>.from(state.draft.sensors);
    if (index < 0 || index >= sensors.length) return;
    sensors[index] = Map<String, dynamic>.from(sensors[index])..['type'] = type;
    _setDraft(state.draft.copyWith(sensors: sensors));
  }

  void updateSensorPin(int index, int pin) {
    final sensors = List<Map<String, dynamic>>.from(state.draft.sensors);
    if (index < 0 || index >= sensors.length) return;
    final sensor = Map<String, dynamic>.from(sensors[index]);
    final bus = Map<String, dynamic>.from(
      (sensor['bus'] as Map<String, dynamic>?) ?? {'type': 'onewire'},
    );
    bus['pin'] = pin;
    sensor['bus'] = bus;
    sensors[index] = sensor;
    _setDraft(state.draft.copyWith(sensors: sensors));
  }

  void updateActuatorPin(int index, int pin) {
    final actuators = List<Map<String, dynamic>>.from(state.draft.actuators);
    if (index < 0 || index >= actuators.length) return;
    final actuator = Map<String, dynamic>.from(actuators[index]);
    final pins = Map<String, dynamic>.from(
      (actuator['pins'] as Map<String, dynamic>?) ?? {},
    );
    pins['pwm'] = pin;
    actuator['pins'] = pins;
    actuators[index] = actuator;
    _setDraft(state.draft.copyWith(actuators: actuators));
  }

  void updateDisplayEnabled(bool enabled) {
    final display = Map<String, dynamic>.from(state.draft.display)..['enabled'] = enabled;
    _setDraft(state.draft.copyWith(display: display));
  }

  void updateDisplayDriver(String driver) {
    final display = Map<String, dynamic>.from(state.draft.display)..['driver'] = driver;
    _setDraft(state.draft.copyWith(display: display));
  }

  void updateDisplayWidth(int width) {
    final display = Map<String, dynamic>.from(state.draft.display);
    final geometry = Map<String, dynamic>.from(
      (display['geometry'] as Map<String, dynamic>?) ?? {},
    );
    geometry['width'] = width;
    display['geometry'] = geometry;
    _setDraft(state.draft.copyWith(display: display));
  }

  void updateDisplayHeight(int height) {
    final display = Map<String, dynamic>.from(state.draft.display);
    final geometry = Map<String, dynamic>.from(
      (display['geometry'] as Map<String, dynamic>?) ?? {},
    );
    geometry['height'] = height;
    display['geometry'] = geometry;
    _setDraft(state.draft.copyWith(display: display));
  }

  Future<void> save() async {
    final validation = PinValidator().validateHardwareConfig(
      state.draft,
      deviceModel: state.deviceModel,
    );
    if (!validation.ok) {
      state = state.copyWith(
        saveState: ConfigSaveState.validationFailed,
        validationErrors: validation.errors,
        clearDeviceError: true,
      );
      return;
    }

    state = state.copyWith(
      saveState: ConfigSaveState.saving,
      validationErrors: const [],
      clearDeviceError: true,
    );
    await ref.read(wsClientProvider).sendHardwareConfig(state.draft);
  }

  void resetDraftFromDevice() {
    final cached = ref.read(wsClientProvider).lastHardwareConfig ?? HardwareConfig.defaults();
    state = ConfigUiState(draft: cached.copyWith());
  }

  void _setDraft(HardwareConfig draft) {
    state = state.copyWith(
      draft: draft,
      saveState: ConfigSaveState.idle,
      validationErrors: const [],
      clearDeviceError: true,
    );
  }
}

final configControllerProvider =
    NotifierProvider<ConfigController, ConfigUiState>(ConfigController.new);
