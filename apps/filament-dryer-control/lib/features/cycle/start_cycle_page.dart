/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../features/cycle/cycle_controller.dart';
import '../../features/profiles/profiles_providers.dart';
import '../../l10n/app_localizations.dart';

class StartCyclePage extends ConsumerStatefulWidget {
  const StartCyclePage({super.key});

  @override
  ConsumerState<StartCyclePage> createState() => _StartCyclePageState();
}

class _StartCyclePageState extends ConsumerState<StartCyclePage> {
  final _tempCtrl = TextEditingController(text: '50');
  final _durationCtrl = TextEditingController(text: '120');
  final _humidityCtrl = TextEditingController(text: '15');
  String? _selectedProfileId;
  String? _error;

  @override
  void dispose() {
    _tempCtrl.dispose();
    _durationCtrl.dispose();
    _humidityCtrl.dispose();
    super.dispose();
  }

  void _applyProfile(FilamentProfile profile) {
    _tempCtrl.text = profile.targetTempC.toStringAsFixed(0);
    _durationCtrl.text = profile.defaultDurationMin.toString();
    _humidityCtrl.text = profile.targetHumidityPct.toStringAsFixed(0);
  }

  StartCycleRequest _buildRequest(List<FilamentProfile> profiles) {
    final temp = double.tryParse(_tempCtrl.text);
    final duration = int.tryParse(_durationCtrl.text);
    final humidity = double.tryParse(_humidityCtrl.text);

    if (_selectedProfileId == null) {
      return StartCycleRequest(
        targetTempC: temp,
        maxDurationMin: duration,
        targetHumidityPct: humidity,
      );
    }

    final profile = profiles.where((p) => p.id == _selectedProfileId).firstOrNull;
    if (profile == null) {
      return StartCycleRequest(
        profileId: _selectedProfileId,
        targetTempC: temp,
        maxDurationMin: duration,
        targetHumidityPct: humidity,
      );
    }

    final overrides = <String, dynamic>{};
    if (temp != null && temp != profile.targetTempC) {
      overrides['targetTempC'] = temp;
    }
    if (duration != null && duration != profile.defaultDurationMin) {
      overrides['maxDurationMin'] = duration;
    }
    if (humidity != null && humidity != profile.targetHumidityPct) {
      overrides['targetHumidityPct'] = humidity;
    }

    if (overrides.isEmpty) {
      return StartCycleRequest(profileId: _selectedProfileId);
    }

    return StartCycleRequest(
      profileId: _selectedProfileId,
      targetTempC: overrides['targetTempC'] as double? ?? temp,
      maxDurationMin: overrides['maxDurationMin'] as int? ?? duration,
      targetHumidityPct: overrides['targetHumidityPct'] as double? ?? humidity,
    );
  }

  Future<void> _start(List<FilamentProfile> profiles) async {
    final request = _buildRequest(profiles);
    final errors = CycleCommandValidator.validateStart(request);
    if (errors.isNotEmpty) {
      setState(() => _error = errors.join('\n'));
      return;
    }
    try {
      await ref.read(cycleControllerProvider).start(request);
      if (mounted) context.pop();
    } catch (e) {
      setState(() => _error = '$e');
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final profilesAsync = ref.watch(profilesListProvider);

    return Scaffold(
      appBar: AppBar(title: Text(l10n.startCycle)),
      body: profilesAsync.when(
        data: (profiles) => Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            children: [
              DropdownButtonFormField<String?>(
                value: _selectedProfileId,
                decoration: InputDecoration(labelText: l10n.selectProfile),
                items: [
                  DropdownMenuItem<String?>(
                    value: null,
                    child: Text(l10n.manualParameters),
                  ),
                  ...profiles.map(
                    (p) => DropdownMenuItem<String?>(
                      value: p.id,
                      child: Text(p.nameEn),
                    ),
                  ),
                ],
                onChanged: (id) {
                  setState(() {
                    _selectedProfileId = id;
                    if (id != null) {
                      final profile =
                          profiles.where((p) => p.id == id).firstOrNull;
                      if (profile != null) _applyProfile(profile);
                    }
                  });
                },
              ),
              const SizedBox(height: 12),
              TextField(
                controller: _tempCtrl,
                decoration: InputDecoration(labelText: l10n.tempCLabel),
                keyboardType: TextInputType.number,
              ),
              TextField(
                controller: _durationCtrl,
                decoration: InputDecoration(labelText: l10n.durationMinLabel),
                keyboardType: TextInputType.number,
              ),
              TextField(
                controller: _humidityCtrl,
                decoration: InputDecoration(labelText: l10n.humidityPctLabel),
                keyboardType: TextInputType.number,
              ),
              if (_error != null) ...[
                const SizedBox(height: 8),
                Text(
                  _error!,
                  style: TextStyle(color: Theme.of(context).colorScheme.error),
                ),
              ],
              const Spacer(),
              FilledButton(
                onPressed: () => _start(profiles),
                child: Text(l10n.startCycle),
              ),
            ],
          ),
        ),
        loading: () => const Center(child: CircularProgressIndicator()),
        error: (e, _) => Center(child: Text('$e')),
      ),
    );
  }
}
