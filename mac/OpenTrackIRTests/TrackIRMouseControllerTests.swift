import CoreGraphics
import Testing
@testable import OpenTrackIR

struct TrackIRMouseControllerTests {

    @Test func firstCentroidSampleDoesNotMoveTheCursor() {
        let step = trackIRMouseStep(
            previousCentroid: nil,
            currentCentroid: CGPoint(x: 10, y: 20),
            isMovementEnabled: true,
            speed: 1.0
        )

        #expect(step.cursorDelta == nil)
        #expect(step.nextCentroid == CGPoint(x: 10, y: 20))
    }

    @Test func disabledMovementClearsTrackedCentroid() {
        let step = trackIRMouseStep(
            previousCentroid: CGPoint(x: 10, y: 20),
            currentCentroid: CGPoint(x: 15, y: 28),
            isMovementEnabled: false,
            speed: 1.5
        )

        #expect(step.cursorDelta == nil)
        #expect(step.nextCentroid == nil)
    }

    @Test func consecutiveCentroidsProduceSignedMouseDelta() {
        let step = trackIRMouseStep(
            previousCentroid: CGPoint(x: 10, y: 20),
            currentCentroid: CGPoint(x: 13, y: 18),
            isMovementEnabled: true,
            speed: 2.0
        )

        #expect(step.cursorDelta == CGPoint(x: 6, y: -4))
        #expect(step.nextCentroid == CGPoint(x: 13, y: 18))
    }

    @Test func missingCentroidResetsTrackedState() {
        let step = trackIRMouseStep(
            previousCentroid: CGPoint(x: 10, y: 20),
            currentCentroid: nil,
            isMovementEnabled: true,
            speed: 1.0
        )

        #expect(step.cursorDelta == nil)
        #expect(step.nextCentroid == nil)
    }
}
