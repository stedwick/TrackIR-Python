import Testing
@testable import OpenTrackIR

struct XKeysFootPedalMonitorTests {

    @Test func xKeysDeviceMatcherRecognizesSupportedPedals() {
        #expect(isMatchingXKeysFootPedal(
            vendorID: 0x05F3,
            productID: 0x042C,
            usagePage: 0x000C,
            usage: 0x0001
        ))
        #expect(isMatchingXKeysFootPedal(
            vendorID: 0x05F3,
            productID: 0x0438,
            usagePage: 0x000C,
            usage: 0x0001
        ))
        #expect(!isMatchingXKeysFootPedal(
            vendorID: 0x05F3,
            productID: 0x9999,
            usagePage: 0x000C,
            usage: 0x0001
        ))
    }

    @Test func xKeysMiddlePedalParserReadsTheThirdBit() {
        #expect(xKeysMiddlePedalPressed(report: [0x00, 0x00, 0x04]))
        #expect(xKeysMiddlePedalPressed(report: [0x00, 0x00, 0x05]))
        #expect(!xKeysMiddlePedalPressed(report: [0x00, 0x00, 0x00]))
        #expect(!xKeysMiddlePedalPressed(report: [0x00, 0x00]))
    }

    @Test func xKeysIndicatorStateUsesStoplightStates() {
        #expect(xKeysIndicatorState(
            isEnabled: false,
            didDetectPedal: false,
            isPressed: false
        ) == .disabled)
        #expect(xKeysIndicatorState(
            isEnabled: true,
            didDetectPedal: false,
            isPressed: false
        ) == .notDetected)
        #expect(xKeysIndicatorState(
            isEnabled: true,
            didDetectPedal: true,
            isPressed: false
        ) == .detectedIdle)
        #expect(xKeysIndicatorState(
            isEnabled: true,
            didDetectPedal: true,
            isPressed: true
        ) == .pressed)
    }

    @Test func xKeysFastMouseMultiplierAppliesOnlyWhilePressed() {
        #expect(trackIRMouseEffectiveSpeed(
            baseSpeed: 20.0,
            isXKeysFastMouseEnabled: false,
            isXKeysPedalPressed: true
        ) == 20.0)
        #expect(trackIRMouseEffectiveSpeed(
            baseSpeed: 20.0,
            isXKeysFastMouseEnabled: true,
            isXKeysPedalPressed: false
        ) == 20.0)
        #expect(trackIRMouseEffectiveSpeed(
            baseSpeed: 20.0,
            isXKeysFastMouseEnabled: true,
            isXKeysPedalPressed: true
        ) == 50.0)
    }
}
