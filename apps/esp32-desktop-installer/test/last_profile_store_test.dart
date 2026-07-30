import 'package:esp32_desktop_installer/persistence/last_profile_store.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';

void main() {
  test('InMemoryLastProfileStore redacts password', () async {
    final store = InMemoryLastProfileStore();
    await store.save(
      DeviceProfile(wifi: WiFiConfig(ssid: 'net', password: 'super-secret')),
    );
    final raw = store.peekRaw()!;
    expect(raw.contains('super-secret'), isFalse);
    expect(raw.contains('***'), isTrue);
    final loaded = await store.load();
    expect(loaded!.wifi.ssid, 'net');
    expect(loaded.wifi.hasPassword, isFalse);
  });
}
