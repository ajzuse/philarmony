/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:go_router/go_router.dart';
import 'package:philarmony_core/philarmony_core.dart';
import 'package:uuid/uuid.dart';

import '../../device/device_interfaces.dart';
import '../../device/session_providers.dart';
import '../../l10n/app_localizations.dart';
import '../../platform/mdns_discovery.dart';
import '../../shell/app_breakpoint.dart';
import '../discover/connection_status.dart';
import 'device_settings_page.dart';

final discoveryProvider = Provider<DeviceDiscovery>((ref) => MdnsDiscovery());

final selectedDeviceIdProvider = StateProvider<String?>((ref) => null);

class DevicesRegistryPage extends ConsumerStatefulWidget {
  const DevicesRegistryPage({super.key});

  @override
  ConsumerState<DevicesRegistryPage> createState() => _DevicesRegistryPageState();
}

class _DevicesRegistryPageState extends ConsumerState<DevicesRegistryPage> {
  final _hostCtrl = TextEditingController();
  final _portCtrl = TextEditingController(text: '80');
  List<DiscoveredDevice> _found = [];
  bool _scanning = false;

  @override
  void initState() {
    super.initState();
    _hostCtrl.addListener(_onFormChanged);
    _portCtrl.addListener(_onFormChanged);
  }

  void _onFormChanged() => setState(() {});

  @override
  void dispose() {
    _hostCtrl.removeListener(_onFormChanged);
    _portCtrl.removeListener(_onFormChanged);
    _hostCtrl.dispose();
    _portCtrl.dispose();
    super.dispose();
  }

  Future<void> _scan() async {
    setState(() => _scanning = true);
    final discovery = ref.read(discoveryProvider);
    final results = await discovery.scan().first;
    if (mounted) {
      setState(() {
        _found = results;
        _scanning = false;
      });
    }
  }

  Future<void> _connectManual() async {
    final device = KnownDevice(
      id: const Uuid().v4(),
      nickname: _hostCtrl.text.trim(),
      host: _hostCtrl.text.trim(),
      port: int.tryParse(_portCtrl.text.trim()) ?? 80,
    );
    await ref.read(deviceSessionActionsProvider).connect(device);
    ref.invalidate(knownDevicesProvider);
  }

  Future<void> _connectDiscovered(DiscoveredDevice d) async {
    final device = KnownDevice(
      id: const Uuid().v4(),
      nickname: d.name,
      host: d.host,
      port: d.port,
    );
    await ref.read(deviceSessionActionsProvider).connect(device);
    ref.invalidate(knownDevicesProvider);
  }

  void _selectDevice(KnownDevice device, {required bool useMasterDetail}) {
    ref.read(selectedDeviceIdProvider.notifier).state = device.id;
    if (!useMasterDetail) {
      context.push('/device-settings/${device.id}');
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final session = ref.watch(deviceSessionProvider);
    final knownAsync = ref.watch(knownDevicesProvider);
    final useMasterDetail = appBreakpointOf(context).useMasterDetail;

    return ListView(
      padding: const EdgeInsets.all(16),
      children: [
        ConnectionStatusChip(state: session.connectionState),
        if (session.connectionState == DeviceConnectionState.connected) ...[
          const SizedBox(height: 8),
          ListTile(
            contentPadding: EdgeInsets.zero,
            title: Text(l10n.materialProfiles),
            subtitle: Text(l10n.materialProfilesSubtitle),
            trailing: const Icon(Icons.chevron_right),
            onTap: () => context.push('/profiles'),
          ),
        ],
        const SizedBox(height: 12),
        Semantics(
          label: l10n.scan,
          button: true,
          child: Row(
            children: [
              FilledButton.icon(
                onPressed: _scanning ? null : _scan,
                icon: const Icon(Icons.search),
                label: Text(l10n.scan),
              ),
              if (_scanning) ...[
                const SizedBox(width: 12),
                const SizedBox(
                  width: 20,
                  height: 20,
                  child: CircularProgressIndicator(strokeWidth: 2),
                ),
              ],
            ],
          ),
        ),
        const SizedBox(height: 16),
        TextField(
          controller: _hostCtrl,
          decoration: InputDecoration(labelText: l10n.manualHost),
        ),
        TextField(
          controller: _portCtrl,
          decoration: InputDecoration(labelText: l10n.manualPort),
          keyboardType: TextInputType.number,
        ),
        const SizedBox(height: 8),
        Semantics(
          label: l10n.connect,
          button: true,
          child: FilledButton(
            onPressed: _hostCtrl.text.trim().isEmpty ? null : _connectManual,
            child: Text(l10n.connect),
          ),
        ),
        const SizedBox(height: 24),
        if (_found.isEmpty && !_scanning)
          Text(l10n.noDevices),
        ..._found.map(
          (d) => ListTile(
            title: Text(d.name),
            subtitle: Text('${d.host}:${d.port}'),
            onTap: () => _connectDiscovered(d),
          ),
        ),
        const Divider(height: 32),
        Text(l10n.savedDevices, style: Theme.of(context).textTheme.titleMedium),
        knownAsync.when(
          data: (devices) {
            final manager = ref.watch(sessionManagerProvider);
            final selectedId = ref.watch(selectedDeviceIdProvider);
            return Column(
              children: devices
                  .map(
                    (d) => Semantics(
                      label: d.nickname,
                      selected: d.id == selectedId,
                      button: true,
                      child: ListTile(
                        selected: d.id == selectedId,
                        title: Text(d.nickname),
                        subtitle: Text(d.wsUri),
                        trailing: ConnectionStatusChip(
                          state: manager.isOnline(d.id)
                              ? manager.connectionStateFor(d.id)
                              : DeviceConnectionState.disconnected,
                        ),
                        onTap: () async {
                          try {
                            await ref
                                .read(deviceSessionActionsProvider)
                                .connect(d);
                            _selectDevice(d, useMasterDetail: useMasterDetail);
                          } on SessionLimitExceeded catch (e) {
                            if (context.mounted) {
                              ScaffoldMessenger.of(context).showSnackBar(
                                SnackBar(content: Text('$e')),
                              );
                            }
                          }
                        },
                        onLongPress: () =>
                            _selectDevice(d, useMasterDetail: useMasterDetail),
                      ),
                    ),
                  )
                  .toList(),
            );
          },
          loading: () => const LinearProgressIndicator(),
          error: (e, _) => Text('$e'),
        ),
      ],
    );
  }
}

class DeviceSettingsDetailPane extends ConsumerWidget {
  const DeviceSettingsDetailPane({super.key, required this.deviceId});

  final String? deviceId;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final l10n = AppLocalizations.of(context)!;
    if (deviceId == null) {
      return Center(child: Text(l10n.selectDevice));
    }
    return DeviceSettingsBody(deviceId: deviceId!);
  }
}
