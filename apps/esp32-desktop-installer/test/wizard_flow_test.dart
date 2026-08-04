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

import 'package:flutter_test/flutter_test.dart';
import 'package:philarmony_core/philarmony_core.dart';

import 'package:esp32_desktop_installer/device/serial_device_detector.dart';
import 'package:esp32_desktop_installer/flash/esptool_firmware_flasher.dart';
import 'package:esp32_desktop_installer/flash/flash_error_hints.dart';
import 'package:esp32_desktop_installer/wizard/installer_session_controller.dart';

void main() {
  test('session blocks next on pin conflict', () {
    final c = InstallerSessionController();
    c.session.step = WizardStep.pins;
    c.updateProfile(
      (p) => p..pinMapping = PinMapping(heaterPwm: 25, exhaustFanPwm: 25),
    );
    expect(c.session.validationErrors, isNotEmpty);
    expect(c.session.canGoNext, isFalse);
  });

  test('import requires password again before canFlash', () {
    final c = InstallerSessionController();
    c.setSelectedPort('/dev/ttyUSB0');
    final json = ProfileStore().exportJson(
      DeviceProfile(wifi: WiFiConfig(ssid: 'n', password: 'secret')),
    );
    c.importProfileJson(json);
    expect(c.profile.wifi.hasPassword, isFalse);
    expect(c.canFlash, isFalse);
  });

  test('device step requires serial port before next', () {
    final c = InstallerSessionController();
    c.session.step = WizardStep.device;
    c.validateCurrentStep();
    expect(c.session.canGoNext, isFalse);
    expect(
      c.session.validationErrors.any((e) => e.toLowerCase().contains('port')),
      isTrue,
    );
    c.setSelectedPort('/dev/ttyUSB0');
    c.validateCurrentStep();
    expect(c.session.canGoNext, isTrue);
  });

  test('wizard journey: device→…→review with valid profile', () {
    final c = InstallerSessionController();
    c.setSelectedPort('/dev/ttyUSB0');
    c.setBaudRate(921600);
    c.updateProfile(
      (p) => p
        ..deviceModel = 'ESP32'
        ..wifi = WiFiConfig(ssid: 'lab', password: 'secret')
        ..pinMapping = PinMapping(heaterPwm: 25, exhaustFanPwm: 26),
    );

    // Walk steps until review
    while (c.session.step != WizardStep.review) {
      c.validateCurrentStep();
      expect(c.session.canGoNext, isTrue,
          reason: 'blocked at ${c.session.step}: ${c.session.validationErrors}');
      c.next();
    }
    expect(c.session.step, WizardStep.review);
    expect(c.canFlash, isTrue);
  });

  test('soft network verify warns when unreachable (never fails flash)', () async {
    final status = await SoftPostFlashVerifier().tryReach(
      host: '127.0.0.1',
      timeout: const Duration(milliseconds: 200),
    );
    // Soft verify must never surface a hard failure enum — warn/ok/skipped only.
    expect(
      {
        NetworkVerifyStatus.warn,
        NetworkVerifyStatus.ok,
        NetworkVerifyStatus.skipped,
      }.contains(status),
      isTrue,
    );
  });

  test('resolveHost prefers static IP', () {
    final host = SoftPostFlashVerifier.resolveHost(
      DeviceProfile(
        wifi: WiFiConfig(
          ssid: 'x',
          password: 'y',
          staticIp: {'ip': '10.0.0.50'},
        ),
      ),
    );
    expect(host, '10.0.0.50');
  });

  test('usb unplug hint and partial-state detection', () {
    const err = 'ProcessException: No such file or directory';
    expect(FlashErrorHints.looksLikeUsbUnplug(err), isTrue);
    expect(FlashErrorHints.suggest(err).toLowerCase(), contains('partial'));
  });

  test('PWM-incapable pin rejected', () {
    final r = PinValidator().validate(
      DeviceProfile(
        deviceModel: 'ESP32',
        pinMapping: PinMapping(heaterPwm: 34),
      ),
    );
    expect(r.ok, isFalse);
  });

  test('flash-reserved pin rejected on ESP32', () {
    final r = PinValidator().validate(
      DeviceProfile(
        deviceModel: 'ESP32',
        pinMapping: PinMapping(heaterPwm: 6),
      ),
    );
    expect(r.ok, isFalse);
    expect(r.errors.any((e) => e.contains('reserved')), isTrue);
  });

  test('wifi step requires non-empty password before next', () {
    final c = InstallerSessionController();
    c.session.step = WizardStep.wifi;
    c.updateProfile(
      (p) => p..wifi = WiFiConfig(ssid: 'lab', password: ''),
    );
    expect(c.session.canGoNext, isFalse);
    expect(
      c.session.validationErrors.any((e) => e.toLowerCase().contains('password')),
      isTrue,
    );
    c.updateProfile(
      (p) => p..wifi = WiFiConfig(ssid: 'lab', password: 'secret'),
    );
    expect(c.session.canGoNext, isTrue);
  });

  test('wifi step validates static IP fields', () {
    final c = InstallerSessionController();
    c.session.step = WizardStep.wifi;
    c.updateProfile(
      (p) => p
        ..wifi = WiFiConfig(
          ssid: 'lab',
          password: 'secret',
          staticIp: {
            'ip': 'bad',
            'gateway': '192.168.1.1',
            'netmask': '255.255.255.0',
            'dns': '',
          },
        ),
    );
    expect(c.session.canGoNext, isFalse);
    expect(
      c.session.validationErrors.any((e) => e.toLowerCase().contains('static')),
      isTrue,
    );
  });

  test('FlashJobState includes preflight', () {
    expect(FlashJobState.values, contains(FlashJobState.preflight));
  });

  test('CP210x/CH340/FTDI USB-UART hints from VID/PID', () {
    expect(
      SerialDeviceDetector.usbUartHint(vid: 0x10C4, pid: 0xEA60),
      contains('CP210'),
    );
    expect(
      SerialDeviceDetector.usbUartHint(vid: 0x1A86, pid: 0x7523),
      contains('CH340'),
    );
    expect(
      SerialDeviceDetector.usbUartHint(vid: 0x0403, pid: 0x6001),
      contains('FT'),
    );
  });
}
