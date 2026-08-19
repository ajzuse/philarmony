/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../../l10n/app_localizations.dart';
import '../history/history_stats.dart';

class CycleResultPage extends ConsumerWidget {
  const CycleResultPage({super.key, required this.cycle});

  final Map<String, dynamic> cycle;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(title: const Text('Cycle complete')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Text(
              cycle['material_name']?.toString() ?? 'Cycle',
              style: Theme.of(context).textTheme.headlineSmall,
            ),
            const SizedBox(height: 8),
            Text('Stop reason: ${cycle['stop_reason'] ?? '—'}'),
            const SizedBox(height: 16),
            CycleStatsRow(cycle: cycle),
            const Spacer(),
            FilledButton(
              onPressed: () => context.go('/history'),
              child: Text(l10n.navHistory),
            ),
            const SizedBox(height: 8),
            OutlinedButton(
              onPressed: () => context.go('/dashboard'),
              child: Text(l10n.navDashboard),
            ),
          ],
        ),
      ),
    );
  }
}
