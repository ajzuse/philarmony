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

import 'device_profile.dart';
import 'flash_job.dart';

enum WizardStep {
  device,
  sensors,
  pins,
  display,
  profiles,
  wifi,
  review,
  flash,
}

class InstallerSession {
  InstallerSession({
    String? sessionId,
    this.step = WizardStep.device,
    this.locale = 'pt-BR',
    DeviceProfile? deviceProfile,
    List<String>? validationErrors,
    this.flashJob,
    this.selectedPortPath,
    this.baudRate = 921600,
    this.customPartitionTablePath,
  })  : sessionId = sessionId ?? DateTime.now().microsecondsSinceEpoch.toString(),
        deviceProfile = deviceProfile ?? DeviceProfile(),
        validationErrors = validationErrors ?? [];

  final String sessionId;
  WizardStep step;
  String locale;
  DeviceProfile deviceProfile;
  List<String> validationErrors;
  FlashJob? flashJob;
  /// USB serial path selected on Device step (required for flash).
  String? selectedPortPath;
  /// Serial baud for esptool flash/probe (FR-001, default 921600).
  int baudRate;
  /// Optional override for partitions.bin (FR-008 custom partition tables).
  String? customPartitionTablePath;

  static const orderedSteps = WizardStep.values;

  bool get canGoNext => validationErrors.isEmpty && !isFlashing;
  bool get isFlashing => flashJob?.isActive == true;

  void goNext() {
    if (!canGoNext) return;
    final i = orderedSteps.indexOf(step);
    if (i < orderedSteps.length - 1) step = orderedSteps[i + 1];
  }

  void goBack() {
    if (isFlashing) return;
    final i = orderedSteps.indexOf(step);
    if (i > 0) step = orderedSteps[i - 1];
  }
}
