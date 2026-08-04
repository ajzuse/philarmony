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
import 'package:philarmony_core/philarmony_core.dart';

import '../../l10n/app_localizations.dart';
import '../installer_session_controller.dart';

class ProfilesStep extends StatelessWidget {
  const ProfilesStep({super.key, required this.controller});
  final InstallerSessionController controller;

  /// Visible list: builtins hidden when a custom profile shadows the same id.
  List<(FilamentProfile, int)> _visible(List<FilamentProfile> list) {
    final shadowed = list
        .where((f) => !f.isBuiltin)
        .map((f) => f.id)
        .toSet();
    final out = <(FilamentProfile, int)>[];
    for (var i = 0; i < list.length; i++) {
      final f = list[i];
      if (f.isBuiltin && shadowed.contains(f.id)) continue;
      out.add((f, i));
    }
    return out;
  }

  Future<void> _edit(
    BuildContext context,
    FilamentProfile? existing,
    int index, {
    bool shadowBuiltin = false,
  }) async {
    final list = controller.profile.filamentProfiles;
    final customs = list.where((f) => !f.isBuiltin).length;
    final isNew = existing == null || shadowBuiltin;
    if (isNew && customs >= FilamentProfileValidator.maxCustom) return;

    final base = existing;
    final namePt = TextEditingController(text: base?.namePt ?? 'Custom');
    final nameEn = TextEditingController(text: base?.nameEn ?? 'Custom');
    final temp = TextEditingController(
      text: (base?.targetTempC ?? 55).toString(),
    );
    final dur = TextEditingController(
      text: (base?.defaultDurationMin ?? 180).toString(),
    );
    final hum = TextEditingController(
      text: (base?.targetHumidityPct ?? 15).toString(),
    );

    final l10n = AppLocalizations.of(context);
    final ok = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text(shadowBuiltin
            ? '${l10n?.overrideBuiltinTitle ?? 'Override builtin'} (${base?.id})'
            : (existing == null
                ? (l10n?.addCustomProfile ?? 'Add custom profile')
                : (l10n?.editCustomProfile ?? 'Edit custom profile'))),
        content: SizedBox(
          width: 360,
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              TextField(
                  controller: namePt,
                  decoration: InputDecoration(
                      labelText: l10n?.namePtLabel ?? 'Name PT')),
              TextField(
                  controller: nameEn,
                  decoration: InputDecoration(
                      labelText: l10n?.nameEnLabel ?? 'Name EN')),
              TextField(
                  controller: temp,
                  decoration: InputDecoration(
                      labelText: l10n?.tempCLabel ?? 'Temp °C'),
                  keyboardType: TextInputType.number),
              TextField(
                  controller: dur,
                  decoration: InputDecoration(
                      labelText: l10n?.durationMinLabel ?? 'Duration min'),
                  keyboardType: TextInputType.number),
              TextField(
                  controller: hum,
                  decoration: InputDecoration(
                      labelText: l10n?.humidityPctLabel ?? 'Humidity %'),
                  keyboardType: TextInputType.number),
            ],
          ),
        ),
        actions: [
          TextButton(
              onPressed: () => Navigator.pop(ctx, false),
              child: Text(l10n?.cancelLabel ?? 'Cancel')),
          FilledButton(
              onPressed: () => Navigator.pop(ctx, true),
              child: Text(l10n?.saveLabel ?? 'Save')),
        ],
      ),
    );
    if (ok != true) return;

    final f = FilamentProfile(
      id: shadowBuiltin
          ? (base?.id ?? 'custom')
          : (existing?.id ?? 'custom-${DateTime.now().millisecondsSinceEpoch}'),
      namePt: namePt.text,
      nameEn: nameEn.text,
      targetTempC: double.tryParse(temp.text) ?? 55,
      defaultDurationMin: int.tryParse(dur.text) ?? 180,
      targetHumidityPct: double.tryParse(hum.text) ?? 15,
      isBuiltin: false,
    );
    final errs = FilamentProfileValidator.validate(
      f,
      customCount: isNew ? customs + 1 : customs,
    );
    if (errs.isNotEmpty) {
      if (context.mounted) {
        ScaffoldMessenger.of(context)
            .showSnackBar(SnackBar(content: Text(errs.join('; '))));
      }
      return;
    }
    if (shadowBuiltin) {
      // Replace any prior custom with same id, keep builtin in list but hidden.
      list.removeWhere((e) => !e.isBuiltin && e.id == f.id);
      list.add(f);
    } else if (existing == null) {
      list.add(f);
    } else {
      list[index] = f;
    }
    controller.updateProfile((p) => p);
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final list = controller.profile.filamentProfiles;
    final customs = list.where((f) => !f.isBuiltin).length;
    final visible = _visible(list);
    return Column(
      children: [
        Expanded(
          child: ListView.builder(
            itemCount: visible.length,
            itemBuilder: (context, i) {
              final (f, index) = visible[i];
              final isShadow = !f.isBuiltin &&
                  list.any((b) => b.isBuiltin && b.id == f.id);
              return ListTile(
                title: Text('${f.nameEn} / ${f.namePt}'),
                subtitle: Text(
                  '${f.targetTempC}°C · ${f.defaultDurationMin} min · ${f.targetHumidityPct}%'
                  '${f.isBuiltin ? ' (builtin)' : isShadow ? ' (overrides builtin)' : ''}',
                ),
                onTap: f.isBuiltin ? null : () => _edit(context, f, index),
                trailing: f.isBuiltin
                    ? IconButton(
                        tooltip: l10n?.overrideBuiltinTitle ??
                            'Override builtin (shadow)',
                        icon: const Icon(Icons.copy_all),
                        onPressed: () =>
                            _edit(context, f, index, shadowBuiltin: true),
                      )
                    : Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          IconButton(
                            icon: const Icon(Icons.edit),
                            onPressed: () => _edit(context, f, index),
                          ),
                          IconButton(
                            icon: const Icon(Icons.delete),
                            onPressed: () {
                              list.removeAt(index);
                              controller.updateProfile((p) => p);
                            },
                          ),
                        ],
                      ),
              );
            },
          ),
        ),
        FilledButton(
          onPressed: customs >= FilamentProfileValidator.maxCustom
              ? null
              : () => _edit(context, null, -1),
          child: Text(l10n?.addCustomProfile ?? 'Add custom profile'),
        ),
      ],
    );
  }
}
