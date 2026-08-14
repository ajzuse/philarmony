/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:io' show Platform;

import 'package:flutter/foundation.dart' show kIsWeb;
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../device/device_interfaces.dart';
import 'background_session_android.dart';
import 'background_session_ios.dart';

class NoOpBackgroundSession implements BackgroundSession {
  @override
  bool get isActive => false;

  @override
  Future<bool> start({required String reason}) async => false;

  @override
  Future<void> stop() async {}
}

final backgroundSessionProvider = Provider<BackgroundSession>((ref) {
  if (kIsWeb) return NoOpBackgroundSession();
  if (Platform.isAndroid) return AndroidBackgroundSession();
  if (Platform.isIOS) return IosBackgroundSession();
  return NoOpBackgroundSession();
});
