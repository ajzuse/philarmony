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

import 'package:flutter/material.dart';

import '../../l10n/app_localizations.dart';
import '../installer_session_controller.dart';

class ReviewStep extends StatelessWidget {
  const ReviewStep({super.key, required this.controller});
  final InstallerSessionController controller;

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final p = controller.profile;
    return ListView(
      children: [
        Text(l10n?.reviewModel(p.deviceModel, p.flashSizeMb) ??
            'Model: ${p.deviceModel} (${p.flashSizeMb} MB)'),
        Text(l10n?.reviewHeaterPwm(p.pinMapping.heaterPwm) ??
            'Heater PWM: ${p.pinMapping.heaterPwm}'),
        Text(l10n?.reviewWifiSsid(p.wifi.ssid) ??
            'WiFi SSID: ${p.wifi.ssid}'),
        Text(l10n?.reviewProfiles(p.filamentProfiles.length) ??
            'Profiles: ${p.filamentProfiles.length}'),
        Text(l10n?.reviewFirmware(p.firmwareVersion) ??
            'Firmware: ${p.firmwareVersion}'),
        const SizedBox(height: 16),
        Text(
          l10n?.reviewEraseWarning ??
              'Install will erase and reflash the device with this configuration. '
                  'There is no automatic restore of a previous firmware image.',
        ),
        if (controller.session.validationErrors.isNotEmpty)
          Text(
            controller.session.validationErrors.join('\n'),
            style: TextStyle(color: Theme.of(context).colorScheme.error),
          ),
      ],
    );
  }
}
