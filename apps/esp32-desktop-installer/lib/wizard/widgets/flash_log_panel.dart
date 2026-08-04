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

import '../../l10n/app_localizations.dart';

class FlashLogPanel extends StatefulWidget {
  const FlashLogPanel({
    super.key,
    required this.lines,
    this.verbose = true,
  });
  final List<String> lines;
  final bool verbose;

  @override
  State<FlashLogPanel> createState() => _FlashLogPanelState();
}

class _FlashLogPanelState extends State<FlashLogPanel> {
  late bool verbose;

  @override
  void initState() {
    super.initState();
    verbose = widget.verbose;
  }

  List<String> get _visible {
    if (verbose) return widget.lines;
    return widget.lines
        .where((l) =>
            l.contains('Flash Success') ||
            l.contains('failed') ||
            l.contains('error') ||
            l.contains('erase') ||
            l.contains('write_flash') ||
            l.contains('verify') ||
            l.contains('preflight'))
        .toList();
  }

  Future<void> _exportFile() async {
    final l10n = AppLocalizations.of(context);
    final path = await FilePicker.platform.saveFile(
      dialogTitle: l10n?.flashLogExportTitle ?? 'Export flash log',
      fileName: 'philarmony-flash.log',
    );
    if (path != null) {
      await File(path).writeAsString(widget.lines.join('\n'));
      return;
    }
    await Clipboard.setData(ClipboardData(text: widget.lines.join('\n')));
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final lines = _visible;
    return Column(
      children: [
        Row(
          children: [
            FilterChip(
              label: Text(
                verbose
                    ? (l10n?.flashLogVerbose ?? 'Verbose')
                    : (l10n?.flashLogQuiet ?? 'Quiet'),
              ),
              selected: verbose,
              onSelected: (v) => setState(() => verbose = v),
            ),
            const Spacer(),
            TextButton.icon(
              onPressed: _exportFile,
              icon: const Icon(Icons.save_alt),
              label: Text(l10n?.flashLogExport ?? 'Export log'),
            ),
          ],
        ),
        Expanded(
          child: Container(
            color: Colors.black87,
            padding: const EdgeInsets.all(8),
            child: ListView.builder(
              itemCount: lines.length,
              itemBuilder: (_, i) => Text(
                lines[i],
                style: const TextStyle(
                  fontFamily: 'monospace',
                  color: Colors.greenAccent,
                  fontSize: 12,
                ),
              ),
            ),
          ),
        ),
      ],
    );
  }
}
