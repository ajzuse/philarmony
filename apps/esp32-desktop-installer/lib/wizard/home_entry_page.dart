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

import 'package:flutter/material.dart';

import '../l10n/app_localizations.dart';
import '../app.dart';
import 'installer_session_controller.dart';
import 'profile_io_actions.dart';
import 'wizard_page.dart';
import '../persistence/last_profile_store.dart';

class HomeEntryPage extends StatelessWidget {
  const HomeEntryPage({super.key});

  void _open(BuildContext context, InstallerSessionController c) {
    Navigator.of(context).push(
      MaterialPageRoute(builder: (_) => WizardPage(controller: c)),
    );
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.appTitle),
        actions: [
          PopupMenuButton<Locale>(
            tooltip: 'Language',
            icon: const Icon(Icons.language),
            onSelected: (locale) => PhilarmonyInstallerApp.setLocale(context, locale),
            itemBuilder: (_) => const [
              PopupMenuItem(value: Locale('pt'), child: Text('Português')),
              PopupMenuItem(value: Locale('en'), child: Text('English')),
            ],
          ),
        ],
      ),
      body: Center(
        child: ConstrainedBox(
          constraints: const BoxConstraints(maxWidth: 420),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              FilledButton(
                onPressed: () => _open(context, InstallerSessionController()),
                child: Text(l10n.newSetup),
              ),
              const SizedBox(height: 12),
              OutlinedButton(
                onPressed: () async {
                  final store = LastProfileStore();
                  final profile = await store.load();
                  if (!context.mounted) return;
                  final c = InstallerSessionController();
                  if (profile != null) c.loadProfile(profile);
                  _open(context, c);
                },
                child: Text(l10n.loadLastSession),
              ),
              const SizedBox(height: 12),
              OutlinedButton(
                onPressed: () async {
                  final c = InstallerSessionController();
                  final err = await ProfileIoActions.importFromPicker(c);
                  if (!context.mounted) return;
                  if (err != null) {
                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(content: Text(err)),
                    );
                    return;
                  }
                  _open(context, c);
                },
                child: Text(l10n.importProfile),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
