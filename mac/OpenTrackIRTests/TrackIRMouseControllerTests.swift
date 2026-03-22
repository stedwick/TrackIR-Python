import CoreGraphics
import Testing
@testable import OpenTrackIR

struct TrackIRMouseControllerTests {

    @Test func firstCentroidSampleDoesNotMoveTheCursor() {
        let step = trackIRMouseStep(
            previousCentroid: nil,
            currentCentroid: CGPoint(x: 10, y: 20),
            isMovementEnabled: true,
            speed: 1.0,
            transform: VideoPreviewTransform(scaleX: 1, scaleY: 1, rotationDegrees: 0)
        )

        #expect(step.cursorDelta == nil)
        #expect(step.nextCentroid == CGPoint(x: 10, y: 20))
    }

    @Test func disabledMovementClearsTrackedCentroid() {
        let step = trackIRMouseStep(
            previousCentroid: CGPoint(x: 10, y: 20),
            currentCentroid: CGPoint(x: 15, y: 28),
            isMovementEnabled: false,
            speed: 1.5,
            transform: VideoPreviewTransform(scaleX: 1, scaleY: 1, rotationDegrees: 0)
        )

        #expect(step.cursorDelta == nil)
        #expect(step.nextCentroid == nil)
    }

    @Test func consecutiveCentroidsProduceSignedMouseDelta() {
        let step = trackIRMouseStep(
            previousCentroid: CGPoint(x: 10, y: 20),
            currentCentroid: CGPoint(x: 13, y: 18),
            isMovementEnabled: true,
            speed: 2.0,
            transform: VideoPreviewTransform(scaleX: 1, scaleY: 1, rotationDegrees: 0)
        )

        #expect(step.cursorDelta == CGPoint(x: 6, y: -4))
        #expect(step.nextCentroid == CGPoint(x: 13, y: 18))
    }

    @Test func missingCentroidResetsTrackedState() {
        let step = trackIRMouseStep(
            previousCentroid: CGPoint(x: 10, y: 20),
            currentCentroid: nil,
            isMovementEnabled: true,
            speed: 1.0,
            transform: VideoPreviewTransform(scaleX: 1, scaleY: 1, rotationDegrees: 0)
        )

        #expect(step.cursorDelta == nil)
        #expect(step.nextCentroid == nil)
    }

    @Test func transformedMouseDeltaAppliesPreviewFlipAndRotation() {
        #expect(transformedTrackIRMouseDelta(
            rawDelta: CGPoint(x: 3, y: -2),
            speed: 2.0,
            transform: VideoPreviewTransform(scaleX: -1, scaleY: 1, rotationDegrees: 0)
        ) == CGPoint(x: -6, y: -4))

        let rotated = transformedTrackIRMouseDelta(
            rawDelta: CGPoint(x: 4, y: 0),
            speed: 1.5,
            transform: VideoPreviewTransform(scaleX: 1, scaleY: 1, rotationDegrees: 90)
        )

        #expect(abs(rotated.x) < 0.0001)
        #expect(abs(rotated.y - 6.0) < 0.0001)
    }

    @Test func clampedCursorPositionStaysOnTheMainDisplayBounds() {
        #expect(trackIRClampedCursorPosition(
            currentPosition: CGPoint(x: 100, y: 100),
            delta: CGPoint(x: 40, y: -25),
            displayBounds: CGRect(x: 0, y: 0, width: 120, height: 120)
        ) == CGPoint(x: 120, y: 75))

        #expect(trackIRClampedCursorPosition(
            currentPosition: CGPoint(x: 10, y: 10),
            delta: CGPoint(x: -30, y: -40),
            displayBounds: CGRect(x: 0, y: 0, width: 120, height: 120)
        ) == CGPoint(x: 0, y: 0))
    }
}
