/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../l10n/app_localizations.dart';
import 'pending_commands_count.dart';

class PendingCommandsBadge extends ConsumerWidget {
  const PendingCommandsBadge({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final count = ref.watch(pendingCommandsCountProvider);
    if (count <= 0) return const SizedBox.shrink();
    final l10n = AppLocalizations.of(context)!;
    return Padding(
      padding: const EdgeInsets.only(right: 4),
      child: Tooltip(
        message: l10n.pendingCommandsTooltip(count),
        child: Badge(
          label: Text('$count'),
          child: const Icon(Icons.cloud_upload_outlined, size: 20),
        ),
      ),
    );
  }
}
