/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../data/app_preferences.dart';
import '../../device/device_interfaces.dart';
import '../../device/session_providers.dart';
import '../../features/cycle/cycle_actions.dart';
import '../../features/cycle/cycle_controller.dart';
import '../../features/discover/connection_status.dart';
import '../../features/profiles/profile_display.dart';
import '../../features/profiles/profiles_providers.dart';
import '../../features/settings/unit_format.dart';
import '../../l10n/app_localizations.dart';
import 'fault_banner.dart';
import 'live_charts.dart';

enum ThermalBadge { idle, heating, cooling }

ThermalBadge resolveThermalBadge(StatusSnapshot status) {
  if (status.status == DryerStatus.idle) return ThermalBadge.idle;
  if (status.heaterOn) return ThermalBadge.heating;
  if (status.exhaustFanOn) return ThermalBadge.cooling;
  return ThermalBadge.idle;
}

class DashboardPage extends ConsumerWidget {
  const DashboardPage({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final session = ref.watch(deviceSessionProvider);
    final status = session.status;
    final l10n = AppLocalizations.of(context)!;
    final prefs = ref.watch(appPreferencesProvider).maybeWhen(
          data: (value) => value,
          orElse: () => null,
        );
    final useCelsius = prefs?.useCelsius ?? true;
    final timeFormat = prefs?.timeFormat ?? AppTimeFormat.h24;
    final chartWindowMinutes = prefs?.chartWindowMinutes ?? 60;
    final readings = ref.watch(lastValidReadingsProvider);

    final buffer = ref.watch(telemetryBufferProvider);
    final temps = buffer.temps;
    final humidity = buffer.humidity;
    final heaterPower = buffer.heaterPower;

    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        Semantics(
          label: 'Connection status ${session.connectionState.name}',
          child: ConnectionStatusChip(state: session.connectionState),
        ),
        if (!session.backgroundAllowed)
          MaterialBanner(
            content: Text(l10n.backgroundNotAllowed),
            leading: const Icon(Icons.warning_amber_rounded),
            actions: [
              TextButton(
                onPressed: () =>
                    ScaffoldMessenger.of(context).hideCurrentMaterialBanner(),
                child: Text(l10n.dismiss),
              ),
            ],
          ),
        if (session.fault != null) FaultBanner(fault: session.fault!),
        const SizedBox(height: 12),
        if (status != null) ...[
          _ThermalBadges(badge: resolveThermalBadge(status)),
          const SizedBox(height: 8),
          if (status.status == DryerStatus.drying)
            _CycleProgressRing(status: status),
          if (status.hasSensorError)
            Padding(
              padding: const EdgeInsets.only(bottom: 8),
              child: Chip(
                label: Text(l10n.sensorError),
                backgroundColor: Colors.amber,
              ),
            ),
          _MetricRow(l10n.statusLabel, status.status.name),
          _MetricRow(
            l10n.chamberTemp,
            formatTemperature(readings.chamberTempC, useCelsius: useCelsius),
          ),
          _MetricRow(
            l10n.targetTemp,
            formatTemperature(status.targetTempC, useCelsius: useCelsius),
          ),
          _MetricRow(
            l10n.humidity,
            formatHumidity(readings.humidityPct),
          ),
          _MetricRow(
            l10n.targetHumidity,
            formatHumidity(status.targetHumidityPct),
          ),
          _MetricRow(
            l10n.heater,
            '${formatOnOff(status.heaterOn)} · ${formatPercent(status.heaterPowerPct)}',
          ),
          _MetricRow(
            l10n.exhaustFan,
            '${formatOnOff(status.exhaustFanOn)} · ${formatPercent(status.exhaustFanPowerPct)}',
          ),
          _MetricRow(
            l10n.elapsedTime,
            formatDuration(status.elapsedTimeSec, timeFormat: timeFormat),
          ),
          _MetricRow(
            l10n.remainingTime,
            formatDuration(status.remainingTimeSec, timeFormat: timeFormat),
          ),
          _MetricRow(l10n.cpuUsage, formatPercent(status.cpuUsagePct)),
          _MetricRow(l10n.memoryFree, formatBytes(status.memoryFreeBytes)),
          _MetricRow(
            l10n.uptime,
            formatDuration(status.uptimeSec, timeFormat: timeFormat),
          ),
        ],
        const SizedBox(height: 16),
        _ProfileQuickStart(
          enabled: session.connectionState == DeviceConnectionState.connected &&
              status?.status != DryerStatus.drying,
        ),
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
        const SizedBox(height: 24),
        LiveCharts(
          temps: temps,
          humidity: humidity,
          heaterPower: heaterPower,
          chartWindowMinutes: chartWindowMinutes,
        ),
      ],
    );
  }
}

class DashboardChartsPane extends ConsumerWidget {
  const DashboardChartsPane({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final chartWindowMinutes = ref.watch(appPreferencesProvider).maybeWhen(
          data: (prefs) => prefs.chartWindowMinutes,
          orElse: () => 60,
        );
    final buffer = ref.watch(telemetryBufferProvider);
    return Padding(
      padding: const EdgeInsets.all(16),
      child: LiveCharts(
        temps: buffer.temps,
        humidity: buffer.humidity,
        heaterPower: buffer.heaterPower,
        chartWindowMinutes: chartWindowMinutes,
      ),
    );
  }
}

class _ThermalBadges extends StatelessWidget {
  const _ThermalBadges({required this.badge});

  final ThermalBadge badge;

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final (label, color) = switch (badge) {
      ThermalBadge.heating => (l10n.heatingBadge, Colors.orange),
      ThermalBadge.cooling => (l10n.coolingBadge, Colors.blue),
      ThermalBadge.idle => (l10n.idleBadge, Colors.grey),
    };
    return Align(
      alignment: Alignment.centerLeft,
      child: Chip(
        label: Text(label),
        backgroundColor: color.withValues(alpha: 0.2),
        side: BorderSide(color: color),
      ),
    );
  }
}

class _CycleProgressRing extends StatelessWidget {
  const _CycleProgressRing({required this.status});

  final StatusSnapshot status;

  @override
  Widget build(BuildContext context) {
    final total = status.elapsedTimeSec + status.remainingTimeSec;
    final progress = total > 0 ? status.elapsedTimeSec / total : 0.0;
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 12),
      child: Center(
        child: SizedBox(
          width: 96,
          height: 96,
          child: Stack(
            alignment: Alignment.center,
            children: [
              CircularProgressIndicator(
                value: progress.clamp(0.0, 1.0),
                strokeWidth: 8,
              ),
              Text('${(progress * 100).toStringAsFixed(0)}%'),
            ],
          ),
        ),
      ),
    );
  }
}

class _ProfileQuickStart extends ConsumerWidget {
  const _ProfileQuickStart({required this.enabled});

  final bool enabled;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final locale = Localizations.localeOf(context);
    final profilesAsync = ref.watch(profilesListProvider);
    final builtins = profilesAsync.maybeWhen(
      data: (profiles) => profiles.where((p) => p.isBuiltin).toList(),
      orElse: () => FilamentProfile.builtins(),
    );

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(l10n.startWithProfile, style: Theme.of(context).textTheme.titleSmall),
        const SizedBox(height: 8),
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: [
            for (final profile in builtins)
              ActionChip(
                label: Text(
                  profileDisplayName(profile, locale),
                ),
                onPressed: !enabled
                    ? null
                    : () async {
                        try {
                          await ref.read(cycleControllerProvider).start(
                                StartCycleRequest(profileId: profile.id),
                              );
                        } catch (e) {
                          if (context.mounted) {
                            ScaffoldMessenger.of(context).showSnackBar(
                              SnackBar(content: Text('$e')),
                            );
                          }
                        }
                      },
              ),
          ],
        ),
      ],
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
