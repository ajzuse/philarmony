/*
 * Philarmony Filament Dryer Control App
 * Copyright (C) 2026 Philarmony Contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import Flutter
import UIKit

@main
@objc class AppDelegate: FlutterAppDelegate, FlutterImplicitEngineDelegate {
  override func application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) -> Bool {
    return super.application(application, didFinishLaunchingWithOptions: launchOptions)
  }

  func didInitializeImplicitFlutterEngine(_ engineBridge: FlutterImplicitEngineBridge) {
    GeneratedPluginRegistrant.register(with: engineBridge.pluginRegistry)

    // Stub: iOS sustained WS relies on UIBackgroundModes (fetch/processing) in
    // Info.plist. No foreground-service equivalent; channel returns true so Dart
    // lifecycle can proceed. Real BGTask scheduling may bind here later.
    let channel = FlutterMethodChannel(
      name: "philarmony/background",
      binaryMessenger: engineBridge.applicationRegistrar.messenger()
    )
    channel.setMethodCallHandler { call, result in
      switch call.method {
      case "start":
        result(true)
      case "stop":
        result(nil)
      default:
        result(FlutterMethodNotImplemented)
      }
    }
  }
}
