/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../device/session_providers.dart';
import '../history/history_controller.dart';

class CycleController {
  CycleController(this._ref);

  final Ref _ref;

  Future<void> start(StartCycleRequest request) async {
    final errors = CycleCommandValidator.validateStart(request);
    if (errors.isNotEmpty) throw ArgumentError(errors.join('; '));
    await _ref.read(deviceSessionProvider.notifier).startCycle(request);
  }

  Future<Map<String, dynamic>?> stop({String stopReason = 'user_stop'}) async {
    final cycleId = _ref.read(deviceSessionProvider).activeCycleId;
    await _ref.read(deviceSessionProvider.notifier).stopCycle();
    _ref.invalidate(historyCyclesProvider);
    if (cycleId == null) return null;
    final repo = await _ref.read(dryingCycleRepositoryProvider.future);
    return repo.byId(cycleId);
  }
}

final cycleControllerProvider = Provider<CycleController>((ref) => CycleController(ref));
