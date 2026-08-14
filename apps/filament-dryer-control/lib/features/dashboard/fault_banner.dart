/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:philarmony_core/philarmony_core.dart';

class FaultBanner extends StatelessWidget {
  const FaultBanner({super.key, required this.fault});

  final FaultEvent fault;

  @override
  Widget build(BuildContext context) {
    return Card(
      color: Theme.of(context).colorScheme.errorContainer,
      child: ListTile(
        leading: const Icon(Icons.warning_amber_rounded),
        title: Text(fault.faultCode),
        subtitle: Text(fault.message),
      ),
    );
  }
}
