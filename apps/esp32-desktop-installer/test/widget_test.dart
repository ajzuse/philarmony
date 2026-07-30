import 'package:flutter_test/flutter_test.dart';

import 'package:esp32_desktop_installer/app.dart';

void main() {
  testWidgets('app launches home entry', (WidgetTester tester) async {
    await tester.pumpWidget(const PhilarmonyInstallerApp());
    expect(find.text('Philarmony Installer'), findsOneWidget);
    expect(find.text('New setup'), findsOneWidget);
  });
}
