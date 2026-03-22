import CoreGraphics

struct TrackIRMouseStep: Equatable {
    let cursorDelta: CGPoint?
    let nextCentroid: CGPoint?
}

func trackIRMouseStep(
    previousCentroid: CGPoint?,
    currentCentroid: CGPoint?,
    isMovementEnabled: Bool,
    speed: Double,
    transform: VideoPreviewTransform
) -> TrackIRMouseStep {
    guard isMovementEnabled, let currentCentroid else {
        return TrackIRMouseStep(cursorDelta: nil, nextCentroid: nil)
    }

    guard let previousCentroid else {
        return TrackIRMouseStep(cursorDelta: nil, nextCentroid: currentCentroid)
    }

    return TrackIRMouseStep(
        cursorDelta: transformedTrackIRMouseDelta(
            rawDelta: CGPoint(
                x: currentCentroid.x - previousCentroid.x,
                y: currentCentroid.y - previousCentroid.y
            ),
            speed: speed,
            transform: transform
        ),
        nextCentroid: currentCentroid
    )
}

@MainActor
final class TrackIRMouseController {
    private var hasRequestedPostEventAccess = false
    private var previousCentroid: CGPoint?

    func update(
        centroidX: Double?,
        centroidY: Double?,
        isMovementEnabled: Bool,
        speed: Double,
        transform: VideoPreviewTransform
    ) {
        let currentCentroid = trackIRCentroidPoint(x: centroidX, y: centroidY)
        let step = trackIRMouseStep(
            previousCentroid: previousCentroid,
            currentCentroid: currentCentroid,
            isMovementEnabled: isMovementEnabled,
            speed: speed,
            transform: transform
        )

        previousCentroid = step.nextCentroid

        guard let cursorDelta = step.cursorDelta else {
            return
        }

        moveCursor(by: cursorDelta)
    }

    func reset() {
        previousCentroid = nil
    }

    private func moveCursor(by delta: CGPoint) {
        guard let currentEvent = CGEvent(source: nil) else {
            return
        }

        let nextPosition = trackIRClampedCursorPosition(
            currentPosition: currentEvent.location,
            delta: delta,
            displayBounds: CGDisplayBounds(CGMainDisplayID())
        )

        if canPostMouseEvents() {
            guard let mouseEvent = CGEvent(
                mouseEventSource: nil,
                mouseType: .mouseMoved,
                mouseCursorPosition: nextPosition,
                mouseButton: .left
            ) else {
                return
            }

            mouseEvent.post(tap: .cghidEventTap)
            return
        }

        // Disabled while testing the pure Quartz post path.
        // CGWarpMouseCursorPosition(nextPosition)
    }

    private func canPostMouseEvents() -> Bool {
        if CGPreflightPostEventAccess() {
            return true
        }

        guard !hasRequestedPostEventAccess else {
            return false
        }

        hasRequestedPostEventAccess = true
        return CGRequestPostEventAccess()
    }
}

func transformedTrackIRMouseDelta(
    rawDelta: CGPoint,
    speed: Double,
    transform: VideoPreviewTransform
) -> CGPoint {
    let flippedX = rawDelta.x * transform.scaleX
    let flippedY = rawDelta.y * transform.scaleY
    let radians = transform.rotationDegrees * .pi / 180.0
    let rotatedX = (flippedX * cos(radians)) - (flippedY * sin(radians))
    let rotatedY = (flippedX * sin(radians)) + (flippedY * cos(radians))

    return CGPoint(
        x: rotatedX * speed,
        y: rotatedY * speed
    )
}

func trackIRClampedCursorPosition(
    currentPosition: CGPoint,
    delta: CGPoint,
    displayBounds: CGRect
) -> CGPoint {
    let unclampedPosition = CGPoint(
        x: currentPosition.x + delta.x,
        y: currentPosition.y + delta.y
    )

    return CGPoint(
        x: min(max(unclampedPosition.x, displayBounds.minX), displayBounds.maxX),
        y: min(max(unclampedPosition.y, displayBounds.minY), displayBounds.maxY)
    )
}

private func trackIRCentroidPoint(x: Double?, y: Double?) -> CGPoint? {
    guard let x, let y else {
        return nil
    }

    return CGPoint(x: x, y: y)
}
