/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:async';

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../device/device_interfaces.dart';
import '../../device/session_providers.dart';
import '../../shell/pending_commands_count.dart';
import 'config_merge_policy.dart';

enum ConfigSaveState { idle, saving, saved, queued, validationFailed, deviceError }

class ConfigUiState {
  const ConfigUiState({
    required this.draft,
    this.deviceModel = 'ESP32',
    this.saveState = ConfigSaveState.idle,
    this.validationErrors = const [],
    this.deviceError,
    this.localEditedAt,
  });

  final HardwareConfig draft;
  final String deviceModel;
  final ConfigSaveState saveState;
  final List<String> validationErrors;
  final String? deviceError;
  final DateTime? localEditedAt;

  ConfigUiState copyWith({
    HardwareConfig? draft,
    String? deviceModel,
    ConfigSaveState? saveState,
    List<String>? validationErrors,
    String? deviceError,
    bool clearDeviceError = false,
    DateTime? localEditedAt,
  }) {
    return ConfigUiState(
      draft: draft ?? this.draft,
      deviceModel: deviceModel ?? this.deviceModel,
      saveState: saveState ?? this.saveState,
      validationErrors: validationErrors ?? this.validationErrors,
      deviceError: clearDeviceError ? null : (deviceError ?? this.deviceError),
      localEditedAt: localEditedAt ?? this.localEditedAt,
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

    final session = ref.read(deviceSessionProvider);
    final cached = session.hardwareCache ??
        client.lastHardwareConfig ??
        HardwareConfig.defaults();
    final model = session.activeDevice?.deviceModel ?? 'ESP32';
    return ConfigUiState(draft: cached.copyWith(), deviceModel: model);
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

  void updateI2cPins({required int sda, required int scl}) {
    final sensors = List<Map<String, dynamic>>.from(state.draft.sensors);
    for (var i = 0; i < sensors.length; i++) {
      final sensor = Map<String, dynamic>.from(sensors[i]);
      final bus = sensor['bus'] as Map<String, dynamic>?;
      if (bus?['type'] == 'i2c') {
        final nextBus = Map<String, dynamic>.from(bus!);
        nextBus['sda_pin'] = sda;
        nextBus['scl_pin'] = scl;
        sensor['bus'] = nextBus;
        sensors[i] = sensor;
      }
    }
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

  void updateHeaterPin(int pin) {
    final index = state.draft.actuators.indexWhere((a) => a['role'] == 'heater');
    if (index >= 0) updateActuatorPin(index, pin);
  }

  void updateFanPin(int pin) {
    final index = state.draft.actuators.indexWhere((a) => a['role'] == 'fan');
    if (index >= 0) updateActuatorPin(index, pin);
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

  void updateDisplayField(String field, bool enabled) {
    final display = Map<String, dynamic>.from(state.draft.display);
    final layout = Map<String, dynamic>.from(
      (display['layout'] as Map<String, dynamic>?) ?? {},
    );
    final fields = List<String>.from(
      (layout['fields'] as List?)?.map((e) => e.toString()) ?? const [],
    );
    if (enabled && !fields.contains(field)) {
      fields.add(field);
    } else if (!enabled) {
      fields.remove(field);
    }
    layout['fields'] = fields;
    display['layout'] = layout;
    _setDraft(state.draft.copyWith(display: display));
  }

  void updateSpiBusPin(String key, int pin) {
    final display = Map<String, dynamic>.from(state.draft.display);
    final bus = Map<String, dynamic>.from(
      (display['bus'] as Map<String, dynamic>?) ?? {'type': 'spi'},
    );
    bus[key] = pin;
    display['bus'] = bus;
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

    final session = ref.read(deviceSessionProvider);
    final deviceId = session.activeDevice?.id;
    if (deviceId == null) return;

    state = state.copyWith(
      saveState: ConfigSaveState.saving,
      validationErrors: const [],
      clearDeviceError: true,
    );

    if (session.connectionState != DeviceConnectionState.connected) {
      final pending = await ref.read(pendingCommandRepositoryProvider.future);
      await pending.enqueue(
        deviceId: deviceId,
        topic: 'config/hardware',
        payload: state.draft.toPayload(),
      );
      ref.read(pendingCommandsCountProvider.notifier).scheduleRefresh();
      state = state.copyWith(saveState: ConfigSaveState.queued);
      return;
    }

    await ref.read(wsClientProvider).sendHardwareConfig(state.draft);
  }

  void resetDraftFromDevice() {
    final client = ref.read(wsClientProvider);
    final session = ref.read(deviceSessionProvider);
    final deviceConfig = session.hardwareCache ?? client.lastHardwareConfig;
    final localEditedAt = state.localEditedAt;
    final deviceUpdatedAt = client.hardwareConfigUpdatedAt;
    if (deviceConfig != null) {
      final resolved = ConfigMergePolicy.resolveHardwareConflict(
        local: state.draft,
        device: deviceConfig,
        deviceUpdatedAt: deviceUpdatedAt,
        localEditedAt: localEditedAt,
      );
      state = ConfigUiState(
        draft: resolved.copyWith(),
        deviceModel: session.activeDevice?.deviceModel ?? state.deviceModel,
      );
      return;
    }
    state = ConfigUiState(
      draft: (client.lastHardwareConfig ?? HardwareConfig.defaults()).copyWith(),
      deviceModel: session.activeDevice?.deviceModel ?? state.deviceModel,
    );
  }

  void _setDraft(HardwareConfig draft) {
    state = state.copyWith(
      draft: draft,
      saveState: ConfigSaveState.idle,
      validationErrors: const [],
      clearDeviceError: true,
      localEditedAt: DateTime.now().toUtc(),
    );
  }
}

final configControllerProvider =
    NotifierProvider<ConfigController, ConfigUiState>(ConfigController.new);
