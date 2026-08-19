/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

final scaffoldMessengerKeyProvider =
    Provider<GlobalKey<ScaffoldMessengerState>>((ref) {
  return GlobalKey<ScaffoldMessengerState>();
});

/// In-app SnackBar / MaterialBanner helper for session alerts.
class AppMessenger {
  AppMessenger(this._messengerKey);

  final GlobalKey<ScaffoldMessengerState> _messengerKey;

  void showSnackBar(String message) {
    final messenger = _messengerKey.currentState;
    if (messenger == null) return;
    messenger.showSnackBar(SnackBar(content: Text(message)));
  }

  void showBanner({
    required String title,
    required String message,
  }) {
    final messenger = _messengerKey.currentState;
    if (messenger == null) return;
    messenger.clearMaterialBanners();
    messenger.showMaterialBanner(
      MaterialBanner(
        content: Text('$title: $message'),
        actions: [
          TextButton(
            onPressed: () => messenger.hideCurrentMaterialBanner(),
            child: const Text('Dismiss'),
          ),
        ],
      ),
    );
  }
}

final appMessengerProvider = Provider<AppMessenger>((ref) {
  return AppMessenger(ref.watch(scaffoldMessengerKeyProvider));
});
