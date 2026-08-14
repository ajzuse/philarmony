import 'package:philarmony_core/philarmony_core.dart';
import 'package:test/test.dart';

void main() {
  test('start requires profile or explicit params', () {
    expect(CycleCommandValidator.validateStart(const StartCycleRequest()), isNotEmpty);
    expect(
      CycleCommandValidator.validateStart(
        const StartCycleRequest(profileId: 'pla'),
      ),
      isEmpty,
    );
  });

  test('start bounds', () {
    final errors = CycleCommandValidator.validateStart(
      const StartCycleRequest(targetTempC: 20, maxDurationMin: 0),
    );
    expect(errors.length, greaterThanOrEqualTo(2));
  });
}
