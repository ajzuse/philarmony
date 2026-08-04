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

import '../../device/wifi_network_scanner.dart';
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
  bool scanning = false;
  List<String> nearbySsids = const [];
  String? scanMessage;
  final _scanner = WifiNetworkScanner();

  @override
  void initState() {
    super.initState();
    useStatic = widget.controller.profile.wifi.staticIp != null;
  }

  Future<void> _scan() async {
    final l10n = AppLocalizations.of(context);
    setState(() {
      scanning = true;
      scanMessage = null;
    });
    final ssids = await _scanner.scanNearbySsids();
    if (!mounted) return;
    setState(() {
      scanning = false;
      nearbySsids = ssids;
      scanMessage = ssids.isEmpty
          ? (l10n?.wifiScanUnavailable ??
              'No networks found (scan unsupported or permission denied). Enter SSID manually.')
          : null;
    });
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final wifi = widget.controller.profile.wifi;
    final staticIp = Map<String, String>.from(wifi.staticIp ?? {});
    final errors = widget.controller.session.validationErrors;
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
        Row(
          children: [
            TextButton.icon(
              onPressed: scanning ? null : _scan,
              icon: scanning
                  ? const SizedBox(
                      width: 16,
                      height: 16,
                      child: CircularProgressIndicator(strokeWidth: 2),
                    )
                  : const Icon(Icons.wifi_find),
              label: Text(l10n?.wifiScanLabel ?? 'Scan nearby WiFi'),
            ),
            if (scanMessage != null)
              Expanded(
                child: Padding(
                  padding: const EdgeInsets.only(left: 8),
                  child: Text(scanMessage!, style: Theme.of(context).textTheme.bodySmall),
                ),
              ),
          ],
        ),
        if (nearbySsids.isNotEmpty)
          Wrap(
            spacing: 8,
            children: nearbySsids
                .take(12)
                .map(
                  (ssid) => ActionChip(
                    label: Text(ssid),
                    onPressed: () {
                      wifi.ssid = ssid;
                      widget.controller.updateProfile((p) => p);
                      setState(() {});
                    },
                  ),
                )
                .toList(),
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
              decoration: InputDecoration(
                labelText: key.toUpperCase(),
                helperText: key == 'dns'
                    ? (l10n?.optionalField ?? 'Optional')
                    : (l10n?.requiredField ?? 'Required'),
              ),
              initialValue: wifi.staticIp?[key] ?? '',
              onChanged: (v) {
                wifi.staticIp ??= {};
                wifi.staticIp![key] = v;
                widget.controller.updateProfile((p) => p);
              },
            ),
        ],
        if (errors.isNotEmpty)
          Padding(
            padding: const EdgeInsets.only(top: 12),
            child: Text(
              errors.join('\n'),
              style: TextStyle(color: Theme.of(context).colorScheme.error),
            ),
          ),
      ],
    );
  }
}
