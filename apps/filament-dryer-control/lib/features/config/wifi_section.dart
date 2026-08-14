/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:url_launcher/url_launcher.dart';

import '../../device/device_runtime_info.dart';
import '../../device/session_providers.dart';
import '../../l10n/app_localizations.dart';

class WifiSection extends ConsumerWidget {
  const WifiSection({super.key});

  static const hotspotSsid = 'philarmony';
  static const hotspotHost = '192.168.4.1';

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final session = ref.watch(deviceSessionProvider);
    final device = session.activeDevice;
    final runtime = session.deviceRuntimeInfo;
    final wifi = resolveWifiPresentation(host: device?.host, runtime: runtime);
    final theme = Theme.of(context);
    final l10n = AppLocalizations.of(context)!;

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(l10n.wifiSettings, style: theme.textTheme.titleMedium),
            if (wifi.hotspotMode) ...[
              const SizedBox(height: 8),
              MaterialBanner(
                content: Text(l10n.wifiHotspotModeBody(hotspotSsid)),
                leading: const Icon(Icons.wifi_tethering),
                backgroundColor: theme.colorScheme.secondaryContainer,
                actions: [
                  TextButton(
                    onPressed: () =>
                        ScaffoldMessenger.of(context).hideCurrentMaterialBanner(),
                    child: Text(l10n.dismiss),
                  ),
                ],
              ),
            ],
            const SizedBox(height: 8),
            ListTile(
              contentPadding: EdgeInsets.zero,
              title: Text(l10n.wifiSsidLabel),
              subtitle: Text(wifi.ssid),
            ),
            ListTile(
              contentPadding: EdgeInsets.zero,
              title: Text(l10n.wifiSignalLabel),
              subtitle: Text(
                wifi.signalDbm == null ? l10n.notReportedMvp : wifi.signalLabel,
              ),
            ),
            const SizedBox(height: 8),
            OutlinedButton.icon(
              onPressed: () => _reconfigureWifi(context, wifi.hotspotMode),
              icon: const Icon(Icons.wifi_tethering),
              label: Text(l10n.wifiReconfigureButton),
            ),
            Text(
              l10n.wifiReconfigureNote,
              style: theme.textTheme.bodySmall,
            ),
          ],
        ),
      ),
    );
  }

  Future<void> _reconfigureWifi(BuildContext context, bool hotspotMode) async {
    final l10n = AppLocalizations.of(context)!;
    if (hotspotMode) {
      final portal = Uri.parse('http://$hotspotHost/');
      if (await canLaunchUrl(portal)) {
        await launchUrl(portal, mode: LaunchMode.externalApplication);
        return;
      }
    }

    if (!context.mounted) return;
    await showDialog<void>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text(l10n.wifiReconfigureTitle),
        content: Text(l10n.wifiReconfigureSteps(hotspotSsid, hotspotHost)),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: Text(l10n.dismiss),
          ),
        ],
      ),
    );
  }
}
