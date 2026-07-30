import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';

import 'package:esp32_desktop_installer/wizard/installer_session_controller.dart';

void main() {
  test('session blocks next on pin conflict', () {
    final c = InstallerSessionController();
    c.session.step = WizardStep.pins;
    c.updateProfile(
      (p) => p
        ..pinMapping = PinMapping(heaterPwm: 25, exhaustFanPwm: 25),
    );
    expect(c.session.validationErrors, isNotEmpty);
    expect(c.session.canGoNext, isFalse);
  });

  test('import requires password again before canFlash', () {
    final c = InstallerSessionController();
    final json = ProfileStore().exportJson(
      DeviceProfile(wifi: WiFiConfig(ssid: 'n', password: 'secret')),
    );
    c.importProfileJson(json);
    expect(c.profile.wifi.hasPassword, isFalse);
    expect(c.canFlash, isFalse);
  });
}
