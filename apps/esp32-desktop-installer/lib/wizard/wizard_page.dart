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
import 'package:philarmony_core/philarmony_core.dart';

import '../l10n/app_localizations.dart';
import 'installer_session_controller.dart';
import 'profile_io_actions.dart';
import 'steps/device_step.dart';
import 'steps/display_step.dart';
import 'steps/flash_step.dart';
import 'steps/pins_step.dart';
import 'steps/profiles_step.dart';
import 'steps/review_step.dart';
import 'steps/sensors_step.dart';
import 'steps/wifi_step.dart';

class WizardPage extends StatefulWidget {
  const WizardPage({super.key, required this.controller});

  final InstallerSessionController controller;

  @override
  State<WizardPage> createState() => _WizardPageState();
}

class _WizardPageState extends State<WizardPage> {
  late final InstallerSessionController c;
  String? importBanner;

  @override
  void initState() {
    super.initState();
    c = widget.controller;
    c.addListener(_onChanged);
  }

  void _onChanged() => setState(() {});

  @override
  void dispose() {
    c.removeListener(_onChanged);
    super.dispose();
  }

  Widget _stepBody() {
    switch (c.session.step) {
      case WizardStep.device:
        return DeviceStep(controller: c);
      case WizardStep.sensors:
        return SensorsStep(controller: c);
      case WizardStep.pins:
        return PinsStep(controller: c);
      case WizardStep.display:
        return DisplayStep(controller: c);
      case WizardStep.profiles:
        return ProfilesStep(controller: c);
      case WizardStep.wifi:
        return WifiStep(controller: c);
      case WizardStep.review:
        return ReviewStep(controller: c);
      case WizardStep.flash:
        return FlashStep(controller: c);
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final errors = c.session.validationErrors;
    return Scaffold(
      appBar: AppBar(
        title: Text('Step: ${c.session.step.name}'),
        actions: [
          IconButton(
            tooltip: l10n.exportProfile,
            onPressed: () => ProfileIoActions.exportProfile(context, c),
            icon: const Icon(Icons.upload_file),
          ),
          IconButton(
            tooltip: l10n.importProfile,
            onPressed: () async {
              final err = await ProfileIoActions.importFromPicker(c);
              setState(() => importBanner = err);
            },
            icon: const Icon(Icons.download),
          ),
        ],
      ),
      body: Column(
        children: [
          if (importBanner != null)
            MaterialBanner(
              content: Text(importBanner!),
              backgroundColor: Theme.of(context).colorScheme.errorContainer,
              actions: [
                TextButton(
                  onPressed: () => setState(() => importBanner = null),
                  child: const Text('Dismiss'),
                ),
              ],
            ),
          if (errors.isNotEmpty)
            MaterialBanner(
              content: Text(errors.join('\n')),
              actions: [
                TextButton(
                  onPressed: () =>
                      ScaffoldMessenger.of(context).hideCurrentMaterialBanner(),
                  child: const Text('OK'),
                ),
              ],
            ),
          Expanded(
            child: Padding(
              padding: const EdgeInsets.all(16),
              child: _stepBody(),
            ),
          ),
          SafeArea(
            child: Padding(
              padding: const EdgeInsets.all(12),
              child: Row(
                children: [
                  OutlinedButton(
                    onPressed: c.session.step == WizardStep.device ||
                            c.session.isFlashing
                        ? null
                        : c.back,
                    child: Text(l10n.back),
                  ),
                  const Spacer(),
                  if (c.session.step != WizardStep.flash)
                    FilledButton(
                      onPressed: errors.isNotEmpty ? null : c.next,
                      child: Text(
                        c.session.step == WizardStep.review
                            ? l10n.install
                            : l10n.next,
                      ),
                    ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}
