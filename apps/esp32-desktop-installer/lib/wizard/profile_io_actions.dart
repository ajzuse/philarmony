/*
 * Philarmony Filament Dryer — Desktop Installer / Shared Core
 * Copyright (C) 2026 Philarmony Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import 'dart:io';

import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'installer_session_controller.dart';

class ProfileIoActions {
  static Future<void> exportProfile(
    BuildContext context,
    InstallerSessionController c,
  ) async {
    final json = c.exportProfileJson();
    final path = await FilePicker.platform.saveFile(
      dialogTitle: 'Export Philarmony profile',
      fileName: 'philarmony-profile.json',
      type: FileType.custom,
      allowedExtensions: const ['json'],
    );
    if (path != null) {
      await File(path).writeAsString(json);
      if (context.mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Saved $path (password redacted)')),
        );
      }
      return;
    }
    await Clipboard.setData(ClipboardData(text: json));
    if (context.mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text('Save cancelled — JSON copied (password redacted)'),
        ),
      );
    }
  }

  static Future<String?> importFromPicker(
    InstallerSessionController c,
  ) async {
    final r = await FilePicker.platform.pickFiles(
      type: FileType.custom,
      allowedExtensions: const ['json'],
    );
    if (r == null || r.files.single.path == null) return null;
    return importFromPath(c, r.files.single.path!);
  }

  static Future<String?> importFromPath(
    InstallerSessionController c,
    String path,
  ) async {
    try {
      final source = await File(path).readAsString();
      c.importProfileJson(source);
      return null;
    } catch (e) {
      return 'Import failed: $e';
    }
  }
}
