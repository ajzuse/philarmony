/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../device/device_interfaces.dart';
import '../../device/session_providers.dart';
import '../../features/discover/connection_status.dart';
import '../../features/cycle/cycle_actions.dart';
import '../../features/settings/unit_format.dart';
import '../../l10n/app_localizations.dart';
import 'fault_banner.dart';
import 'live_charts.dart';

class DashboardPage extends ConsumerWidget {
  const DashboardPage({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final session = ref.watch(deviceSessionProvider);
    final status = session.status;
    final l10n = AppLocalizations.of(context)!;

    final useCelsius = ref.watch(appPreferencesProvider).maybeWhen(
          data: (prefs) => prefs.useCelsius,
          orElse: () => true,
        );

    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Semantics(
          label: 'Connection status ${session.connectionState.name}',
          child: ConnectionStatusChip(state: session.connectionState),
        ),
        if (session.fault != null) FaultBanner(fault: session.fault!),
        const SizedBox(height: 12),
        if (status != null) ...[
          _MetricRow('Status', status.status.name),
          _MetricRow('Temp', formatTemperature(status.chamberTempC, useCelsius: useCelsius)),
          _MetricRow('Target', formatTemperature(status.targetTempC, useCelsius: useCelsius)),
          _MetricRow('Humidity', formatHumidity(status.humidityPct)),
          _MetricRow('Heater', '${status.heaterPowerPct.toStringAsFixed(0)} %'),
          if (status.hasSensorError)
            const Chip(label: Text('Sensor Error'), backgroundColor: Colors.amber),
        ],
        const SizedBox(height: 16),
        Row(
          children: [
            FilledButton(
              onPressed: session.connectionState == DeviceConnectionState.connected
                  ? () => context.push('/start-cycle')
                  : null,
              child: Text(l10n.startCycle),
            ),
            const SizedBox(width: 8),
            if (status?.status == DryerStatus.drying)
              OutlinedButton(
                onPressed: () => confirmAndStopCycle(context, ref),
                child: Text(l10n.stopCycle),
              ),
          ],
        ),
      ],
    );
  }
}

class DashboardChartsPane extends ConsumerWidget {
  const DashboardChartsPane({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    // Select only telemetry temps so status ticks do not rebuild the chart pane.
    final temps = ref.watch(
      telemetryBufferProvider.select((b) => List<double>.from(b.temps)),
    );
    return Padding(
      padding: const EdgeInsets.all(16),
      child: LiveCharts(temps: temps),
    );
  }
}

class _MetricRow extends StatelessWidget {
  const _MetricRow(this.label, this.value);
  final String label;
  final String value;

  @override
  Widget build(BuildContext context) {
    return ListTile(
      dense: true,
      title: Text(label),
      trailing: Text(value, style: Theme.of(context).textTheme.titleMedium),
    );
  }
}
