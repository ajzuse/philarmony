/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'dart:io' show Platform;

import 'package:flutter/material.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:url_launcher/url_launcher.dart';

/// Minimum firmware version supported by this control app build.
const minSupportedFirmware = '0.1.0';

bool isFirmwareMismatch(String? deviceVersion) {
  if (deviceVersion == null || deviceVersion.trim().isEmpty) return false;
  return _compareSemver(deviceVersion.trim(), minSupportedFirmware) < 0;
}

int _compareSemver(String a, String b) {
  List<int> parse(String v) {
    final core = v.split('+').first.split('-').first;
    return core
        .split('.')
        .map((p) => int.tryParse(p) ?? 0)
        .toList();
  }

  final av = parse(a);
  final bv = parse(b);
  final len = av.length > bv.length ? av.length : bv.length;
  for (var i = 0; i < len; i++) {
    final ai = i < av.length ? av[i] : 0;
    final bi = i < bv.length ? bv[i] : 0;
    if (ai != bi) return ai.compareTo(bi);
  }
  return 0;
}

Uri installerLaunchUri() {
  if (Platform.isMacOS) {
    return Uri.parse('file:///Applications/Philarmony%20Installer.app');
  }
  if (Platform.isWindows) {
    return Uri.parse('file:///C:/Program%20Files/Philarmony%20Installer/philarmony_installer.exe');
  }
  if (Platform.isLinux) {
    return Uri.parse('file:///usr/bin/philarmony-installer');
  }
  return Uri.parse('https://github.com/philarmony/philarmony');
}

Future<void> openInstallerApp() async {
  final uri = installerLaunchUri();
  if (await canLaunchUrl(uri)) {
    await launchUrl(uri);
    return;
  }
  final fallback = Uri.parse('https://github.com/philarmony/philarmony');
  if (await canLaunchUrl(fallback)) {
    await launchUrl(fallback, mode: LaunchMode.externalApplication);
  }
}

Future<void> showFirmwareMismatchDialog(
  BuildContext context, {
  required KnownDevice device,
}) async {
  final version = device.firmwareVersion ?? 'unknown';
  if (!context.mounted) return;
  await showDialog<void>(
    context: context,
    builder: (ctx) => AlertDialog(
      title: const Text('Firmware version mismatch'),
      content: Text(
        'Device firmware $version is older than the minimum supported '
        'version $minSupportedFirmware. Open the desktop installer to update '
        'firmware for ${device.nickname}.',
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(ctx),
          child: const Text('Dismiss'),
        ),
        FilledButton(
          onPressed: () async {
            Navigator.pop(ctx);
            await openInstallerApp();
          },
          child: const Text('Open installer'),
        ),
      ],
    ),
  );
}
