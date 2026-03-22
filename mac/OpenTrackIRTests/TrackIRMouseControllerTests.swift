import Testing
@testable import OpenTrackIR

struct TrackIRMouseControllerTests {

    @Test func firstCentroidSampleDoesNotMoveTheCursor() {
        let step = otir_trackir_mouse_compute_step(
            false,
            otir_trackir_mouse_point(x: 0, y: 0),
            true,
            otir_trackir_mouse_point(x: 10, y: 20),
            true,
            1.0,
            otir_trackir_mouse_transform(scale_x: 1, scale_y: 1, rotation_degrees: 0)
        )

        #expect(!step.has_cursor_delta)
        #expect(step.has_next_centroid)
        #expect(step.next_centroid.x == 10)
        #expect(step.next_centroid.y == 20)
    }

    @Test func disabledMovementClearsTrackedCentroid() {
        let step = otir_trackir_mouse_compute_step(
            true,
            otir_trackir_mouse_point(x: 10, y: 20),
            true,
            otir_trackir_mouse_point(x: 15, y: 28),
            false,
            1.5,
            otir_trackir_mouse_transform(scale_x: 1, scale_y: 1, rotation_degrees: 0)
        )

        #expect(!step.has_cursor_delta)
        #expect(!step.has_next_centroid)
    }

    @Test func consecutiveCentroidsProduceSignedMouseDelta() {
        let step = otir_trackir_mouse_compute_step(
            true,
            otir_trackir_mouse_point(x: 10, y: 20),
            true,
            otir_trackir_mouse_point(x: 13, y: 18),
            true,
            2.0,
            otir_trackir_mouse_transform(scale_x: 1, scale_y: 1, rotation_degrees: 0)
        )

        #expect(step.has_cursor_delta)
        #expect(step.cursor_delta.x == 6)
        #expect(step.cursor_delta.y == -4)
        #expect(step.has_next_centroid)
        #expect(step.next_centroid.x == 13)
        #expect(step.next_centroid.y == 18)
    }

    @Test func missingCentroidResetsTrackedState() {
        let step = otir_trackir_mouse_compute_step(
            true,
            otir_trackir_mouse_point(x: 10, y: 20),
            false,
            otir_trackir_mouse_point(x: 0, y: 0),
            true,
            1.0,
            otir_trackir_mouse_transform(scale_x: 1, scale_y: 1, rotation_degrees: 0)
        )

        #expect(!step.has_cursor_delta)
        #expect(!step.has_next_centroid)
    }

    @Test func transformedMouseDeltaAppliesPreviewFlipAndRotation() {
        let flipped = otir_trackir_mouse_transform_delta(
            otir_trackir_mouse_point(x: 3, y: -2),
            2.0,
            otir_trackir_mouse_transform(scale_x: -1, scale_y: 1, rotation_degrees: 0)
        )
        let rotated = otir_trackir_mouse_transform_delta(
            otir_trackir_mouse_point(x: 4, y: 0),
            1.5,
            otir_trackir_mouse_transform(scale_x: 1, scale_y: 1, rotation_degrees: 90)
        )

        #expect(flipped.x == -6)
        #expect(flipped.y == -4)
        #expect(abs(rotated.x) < 0.0001)
        #expect(abs(rotated.y - 6.0) < 0.0001)
    }

    @Test func zeroDeltaDoesNotProduceACursorMove() {
        let step = otir_trackir_mouse_compute_step(
            true,
            otir_trackir_mouse_point(x: 100, y: 120),
            true,
            otir_trackir_mouse_point(x: 100, y: 120),
            true,
            3.0,
            otir_trackir_mouse_transform(scale_x: 1, scale_y: 1, rotation_degrees: 0)
        )

        #expect(!step.has_cursor_delta)
        #expect(step.has_next_centroid)
        #expect(otir_trackir_mouse_point_is_zero(step.cursor_delta))
    }
}
