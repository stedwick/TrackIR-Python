import CoreGraphics
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
        #expect(step.cursor_delta.y == -5)
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

    @Test func trackerSuppressesDeadzoneAndAppliesAdaptiveSmoothing() {
        var state = otir_trackir_mouse_tracker_state()
        let config = otir_trackir_mouse_tracker_config(
            is_movement_enabled: true,
            speed: 10,
            smoothing: 3,
            deadzone: 0.04,
            avoid_mouse_jumps: true,
            jump_threshold_pixels: 50,
            use_adaptive_ema: false,
            use_alpha_beta_filter: false,
            use_quantization_residual_carry: false,
            transform: otir_trackir_mouse_transform(scale_x: 1, scale_y: 1, rotation_degrees: 0)
        )

        let first = otir_trackir_mouse_tracker_update(
            &state,
            true,
            otir_trackir_mouse_point(x: 10, y: 10),
            config
        )
        let deadzone = otir_trackir_mouse_tracker_update(
            &state,
            true,
            otir_trackir_mouse_point(x: 10.02, y: 10.01),
            config
        )
        let smoothed = otir_trackir_mouse_tracker_update(
            &state,
            true,
            otir_trackir_mouse_point(x: 10.52, y: 10.01),
            config
        )

        #expect(!first.has_cursor_delta)
        #expect(!deadzone.has_cursor_delta)
        #expect(smoothed.has_cursor_delta)
        #expect(abs(smoothed.cursor_delta.x - 2.6) < 0.0001)
        #expect(abs(smoothed.cursor_delta.y - 0.0625) < 0.0001)
    }

    @Test func trackerSkipsConfiguredJumpGlitches() {
        var state = otir_trackir_mouse_tracker_state()
        let config = otir_trackir_mouse_tracker_config(
            is_movement_enabled: true,
            speed: 10,
            smoothing: 3,
            deadzone: 0.04,
            avoid_mouse_jumps: true,
            jump_threshold_pixels: 50,
            use_adaptive_ema: false,
            use_alpha_beta_filter: false,
            use_quantization_residual_carry: false,
            transform: otir_trackir_mouse_transform(scale_x: 1, scale_y: 1, rotation_degrees: 0)
        )

        _ = otir_trackir_mouse_tracker_update(
            &state,
            true,
            otir_trackir_mouse_point(x: 10, y: 10),
            config
        )
        let jump = otir_trackir_mouse_tracker_update(
            &state,
            true,
            otir_trackir_mouse_point(x: 61, y: 10),
            config
        )

        #expect(!jump.has_cursor_delta)
    }

    @Test func nativeCursorTargetingQuantizesAndSkipsNoOpPosts() {
        let current = otir_mac_quantized_cursor_position(CGPoint(x: 100.4, y: 200.6))
        let next = otir_mac_clamped_cursor_position_for_bounds(
            current,
            CGPoint(x: 0.2, y: 0.2),
            CGRect(x: 0, y: 0, width: 1_920, height: 1_080)
        )
        let clamped = otir_mac_clamped_cursor_position_for_bounds(
            CGPoint(x: 1_919.4, y: 1_079.4),
            CGPoint(x: 10, y: 10),
            CGRect(x: 0, y: 0, width: 1_920, height: 1_080)
        )

        #expect(current == CGPoint(x: 100, y: 201))
        #expect(next == CGPoint(x: 100, y: 201))
        #expect(!otir_mac_should_post_cursor_move(current, next))
        #expect(clamped == CGPoint(x: 1_920, y: 1_080))
    }
}
