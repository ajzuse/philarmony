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

import 'package:flutter/foundation.dart';
import 'package:philarmony_core/philarmony_core.dart';

/// Wizard session state for the installer UI.
class InstallerSessionController extends ChangeNotifier {
  InstallerSessionController({
    InstallerSession? session,
    PinValidator? pinValidator,
    ProfileStore? profileStore,
  })  : session = session ?? InstallerSession(),
        _pinValidator = pinValidator ?? PinValidator(),
        _profileStore = profileStore ?? ProfileStore();

  final InstallerSession session;
  final PinValidator _pinValidator;
  final ProfileStore _profileStore;

  DeviceProfile get profile => session.deviceProfile;

  void validateCurrentStep() {
    session.validationErrors = [];
    switch (session.step) {
      case WizardStep.pins:
      case WizardStep.review:
      case WizardStep.flash:
        final r = _pinValidator.validate(profile);
        if (!r.ok) session.validationErrors = r.errors;
        break;
      case WizardStep.wifi:
        if (profile.wifi.ssid.trim().isEmpty) {
          session.validationErrors = ['SSID required'];
        }
        break;
      default:
        break;
    }
    notifyListeners();
  }

  bool get canFlash {
    validateCurrentStep();
    if (session.validationErrors.isNotEmpty) return false;
    if (profile.wifi.ssid.isEmpty) return false;
    if (!profile.wifi.hasPassword) {
      session.validationErrors = ['WiFi password required before flash'];
      return false;
    }
    if (session.selectedPortPath == null ||
        session.selectedPortPath!.trim().isEmpty) {
      session.validationErrors = ['Select a USB serial port on the Device step'];
      return false;
    }
    return true;
  }

  void next() {
    validateCurrentStep();
    if (!session.canGoNext) {
      notifyListeners();
      return;
    }
    session.goNext();
    validateCurrentStep();
    notifyListeners();
  }

  void back() {
    session.goBack();
    validateCurrentStep();
    notifyListeners();
  }

  void goTo(WizardStep step) {
    if (session.isFlashing) return;
    session.step = step;
    validateCurrentStep();
    notifyListeners();
  }

  void updateProfile(DeviceProfile Function(DeviceProfile) fn) {
    session.deviceProfile = fn(profile);
    validateCurrentStep();
    notifyListeners();
  }

  String exportProfileJson() => _profileStore.exportJson(profile);

  void importProfileJson(String source) {
    session.deviceProfile = _profileStore.importJson(source);
    session.step = WizardStep.wifi;
    validateCurrentStep();
    notifyListeners();
  }

  void loadProfile(DeviceProfile p) {
    session.deviceProfile = p;
    // Password never restored from disk.
    session.deviceProfile.wifi.password = '';
    session.step = WizardStep.device;
    validateCurrentStep();
    notifyListeners();
  }

  void setSelectedPort(String? path) {
    session.selectedPortPath = path;
    notifyListeners();
  }

  String? get selectedPortPath => session.selectedPortPath;

  void setFlashJob(FlashJob? job) {
    session.flashJob = job;
    notifyListeners();
  }
}
