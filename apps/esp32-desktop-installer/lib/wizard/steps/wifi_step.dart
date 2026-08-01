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

class WifiStep extends StatefulWidget {
  const WifiStep({super.key, required this.controller});
  final InstallerSessionController controller;

  @override
  State<WifiStep> createState() => _WifiStepState();
}

class _WifiStepState extends State<WifiStep> {
  bool show = false;
  bool useStatic = false;

  @override
  void initState() {
    super.initState();
    useStatic = widget.controller.profile.wifi.staticIp != null;
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final wifi = widget.controller.profile.wifi;
    final staticIp = Map<String, String>.from(wifi.staticIp ?? {});
    return ListView(
      children: [
        TextFormField(
          decoration: InputDecoration(
            labelText: '${l10n?.ssidLabel ?? 'SSID'} *',
            helperText: l10n?.requiredField ?? 'Required',
          ),
          initialValue: wifi.ssid,
          onChanged: (v) {
            wifi.ssid = v;
            widget.controller.updateProfile((p) => p);
          },
        ),
        TextFormField(
          decoration: InputDecoration(
            labelText: '${l10n?.passwordLabel ?? 'Password'} *',
            helperText: l10n?.wifiPasswordRequired ?? 'Required before flash',
            suffixIcon: IconButton(
              icon: Icon(show ? Icons.visibility_off : Icons.visibility),
              onPressed: () => setState(() => show = !show),
            ),
          ),
          obscureText: !show,
          onChanged: (v) {
            wifi.password = v;
            widget.controller.updateProfile((p) => p);
          },
        ),
        if (!wifi.hasPassword)
          Padding(
            padding: const EdgeInsets.only(top: 8),
            child: Text(l10n?.wifiPasswordRequired ??
                'Re-enter WiFi password before flashing.'),
          ),
        SwitchListTile(
          title: Text(l10n?.staticIpTitle ?? 'Static IP (optional)'),
          value: useStatic,
          onChanged: (v) {
            setState(() => useStatic = v);
            wifi.staticIp = v
                ? {
                    'ip': staticIp['ip'] ?? '',
                    'gateway': staticIp['gateway'] ?? '',
                    'netmask': staticIp['netmask'] ?? '255.255.255.0',
                    'dns': staticIp['dns'] ?? '',
                  }
                : null;
            widget.controller.updateProfile((p) => p);
          },
        ),
        if (useStatic) ...[
          for (final key in ['ip', 'gateway', 'netmask', 'dns'])
            TextFormField(
              decoration: InputDecoration(labelText: key.toUpperCase()),
              initialValue: wifi.staticIp?[key] ?? '',
              onChanged: (v) {
                wifi.staticIp ??= {};
                wifi.staticIp![key] = v;
                widget.controller.updateProfile((p) => p);
              },
            ),
        ],
      ],
    );
  }
}
