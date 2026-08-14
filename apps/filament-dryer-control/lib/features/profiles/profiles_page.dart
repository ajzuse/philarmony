/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:philarmony_core/philarmony_core.dart';

import '../../l10n/app_localizations.dart';
import 'profiles_providers.dart';

class ProfilesPage extends ConsumerWidget {
  const ProfilesPage({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    final profilesAsync = ref.watch(profilesListProvider);

    return Scaffold(
      appBar: AppBar(title: Text(l10n.materialProfiles)),
      body: profilesAsync.when(
        data: (profiles) => _ProfilesBody(profiles: profiles),
        loading: () => const Center(child: CircularProgressIndicator()),
        error: (e, _) => Center(child: Text('$e')),
      ),
      floatingActionButton: FloatingActionButton.extended(
        onPressed: () => editProfile(context, ref),
        icon: const Icon(Icons.add),
        label: Text(l10n.addCustomProfile),
      ),
    );
  }
}

Future<void> editProfile(
  BuildContext context,
  WidgetRef ref, {
  FilamentProfile? existing,
}) async {
  final l10n = AppLocalizations.of(context)!;
  final svc = await ref.read(profileSyncServiceProvider.future);
  final profiles = await svc.listProfiles();
  final customs = profiles.where((p) => !p.isBuiltin).length;
  final isNew = existing == null;
  if (isNew && customs >= FilamentProfileValidator.maxCustom) {
    if (context.mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(l10n.maxCustomProfiles)),
      );
    }
    return;
  }

  final base = existing;
  final namePt = TextEditingController(text: base?.namePt ?? 'Custom');
  final nameEn = TextEditingController(text: base?.nameEn ?? 'Custom');
  final temp = TextEditingController(text: (base?.targetTempC ?? 55).toString());
  final dur = TextEditingController(
    text: (base?.defaultDurationMin ?? 180).toString(),
  );
  final hum = TextEditingController(
    text: (base?.targetHumidityPct ?? 15).toString(),
  );

  final ok = await showDialog<bool>(
    context: context,
    builder: (ctx) => AlertDialog(
      title: Text(isNew ? l10n.addCustomProfile : l10n.editCustomProfile),
      content: SizedBox(
        width: 360,
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            TextField(
              controller: namePt,
              decoration: InputDecoration(labelText: l10n.namePtLabel),
            ),
            TextField(
              controller: nameEn,
              decoration: InputDecoration(labelText: l10n.nameEnLabel),
            ),
            TextField(
              controller: temp,
              decoration: InputDecoration(labelText: l10n.tempCLabel),
              keyboardType: TextInputType.number,
            ),
            TextField(
              controller: dur,
              decoration: InputDecoration(labelText: l10n.durationMinLabel),
              keyboardType: TextInputType.number,
            ),
            TextField(
              controller: hum,
              decoration: InputDecoration(labelText: l10n.humidityPctLabel),
              keyboardType: TextInputType.number,
            ),
          ],
        ),
      ),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(ctx, false),
          child: Text(l10n.cancel),
        ),
        FilledButton(
          onPressed: () => Navigator.pop(ctx, true),
          child: Text(l10n.saveLabel),
        ),
      ],
    ),
  );
  if (ok != true) return;

  final profile = FilamentProfile(
    id: existing?.id ?? 'custom-${DateTime.now().millisecondsSinceEpoch}',
    namePt: namePt.text,
    nameEn: nameEn.text,
    targetTempC: double.tryParse(temp.text) ?? 55,
    defaultDurationMin: int.tryParse(dur.text) ?? 180,
    targetHumidityPct: double.tryParse(hum.text) ?? 15,
    isBuiltin: false,
  );
  final errs = FilamentProfileValidator.validate(
    profile,
    customCount: isNew ? customs + 1 : customs,
  );
  if (errs.isNotEmpty) {
    if (context.mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(errs.join('; '))),
      );
    }
    return;
  }

  if (isNew) {
    await svc.createProfile(profile);
  } else {
    await svc.updateProfile(profile);
  }
  ref.invalidate(profilesListProvider);
}

class _ProfilesBody extends ConsumerWidget {
  const _ProfilesBody({required this.profiles});

  final List<FilamentProfile> profiles;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    if (profiles.isEmpty) {
      return Center(child: Text(l10n.noProfiles));
    }

    return ListView.separated(
      padding: const EdgeInsets.all(16),
      itemCount: profiles.length,
      separatorBuilder: (_, __) => const Divider(height: 1),
      itemBuilder: (context, index) {
        final profile = profiles[index];
        final subtitle =
            '${profile.targetTempC.toStringAsFixed(0)}°C · '
            '${profile.defaultDurationMin} min · '
            '${profile.targetHumidityPct.toStringAsFixed(0)}%';
        return ListTile(
          title: Text(profile.nameEn),
          subtitle: Text(
            profile.isBuiltin ? '$subtitle · ${l10n.builtinProfile}' : subtitle,
          ),
          trailing: profile.isBuiltin
              ? const Icon(Icons.lock_outline, size: 20)
              : Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    IconButton(
                      icon: const Icon(Icons.edit_outlined),
                      onPressed: () => editProfile(context, ref, existing: profile),
                    ),
                    IconButton(
                      icon: const Icon(Icons.delete_outline),
                      onPressed: () => deleteProfile(context, ref, profile),
                    ),
                  ],
                ),
        );
      },
    );
  }
}

Future<void> deleteProfile(
  BuildContext context,
  WidgetRef ref,
  FilamentProfile profile,
) async {
  final l10n = AppLocalizations.of(context)!;
  final ok = await showDialog<bool>(
    context: context,
    builder: (ctx) => AlertDialog(
      title: Text(l10n.deleteProfileTitle),
      content: Text(l10n.deleteProfileBody(profile.nameEn)),
      actions: [
        TextButton(
          onPressed: () => Navigator.pop(ctx, false),
          child: Text(l10n.cancel),
        ),
        FilledButton(
          onPressed: () => Navigator.pop(ctx, true),
          child: Text(l10n.confirm),
        ),
      ],
    ),
  );
  if (ok != true) return;
  final svc = await ref.read(profileSyncServiceProvider.future);
  await svc.deleteProfile(profile.id);
  ref.invalidate(profilesListProvider);
}
