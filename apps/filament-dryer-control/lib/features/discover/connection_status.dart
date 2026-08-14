/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';

import '../../device/device_interfaces.dart';
import '../../l10n/app_localizations.dart';

class ConnectionStatusChip extends StatelessWidget {
  const ConnectionStatusChip({super.key, required this.state});

  final DeviceConnectionState state;

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final (label, color) = switch (state) {
      DeviceConnectionState.connected => (l10n.connected, Colors.green),
      DeviceConnectionState.connecting || DeviceConnectionState.reconnecting =>
        (l10n.connecting, Colors.orange),
      DeviceConnectionState.error => ('Error', Colors.red),
      _ => (l10n.disconnected, Colors.grey),
    };
    return Chip(
      avatar: CircleAvatar(backgroundColor: color, radius: 6),
      label: Text(label),
    );
  }
}
