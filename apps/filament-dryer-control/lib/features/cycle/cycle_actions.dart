/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';

import '../../features/cycle/cycle_controller.dart';
import '../../l10n/app_localizations.dart';

Future<void> confirmAndStopCycle(BuildContext context, WidgetRef ref) async {
  final l10n = AppLocalizations.of(context)!;
  final ok = await showDialog<bool>(
    context: context,
    builder: (ctx) => AlertDialog(
      title: Text(l10n.stopConfirmTitle),
      content: Text(l10n.stopConfirmBody),
      actions: [
        TextButton(onPressed: () => Navigator.pop(ctx, false), child: Text(l10n.cancel)),
        FilledButton(onPressed: () => Navigator.pop(ctx, true), child: Text(l10n.confirm)),
      ],
    ),
  );
  if (ok == true && context.mounted) {
    final cycle = await ref.read(cycleControllerProvider).stop();
    if (context.mounted && cycle != null) {
      context.push('/cycle-result', extra: cycle);
    }
  }
}
