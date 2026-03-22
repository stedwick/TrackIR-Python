import Combine
import CoreGraphics
import Foundation
import OSLog

enum TrackIRCameraPhase: Equatable {
    case idle
    case starting
    case streaming
    case unavailable
    case failed
}

struct TrackIRCameraDisplayState: Equatable {
    var phase: TrackIRCameraPhase = .idle
    var lastErrorDescription: String?
    var frameIndex: UInt64 = 0
    var frameRate: Double?
    var centroidX: Double?
    var centroidY: Double?
    var lastPacketType: UInt8?
}

@MainActor
final class TrackIRCameraController: ObservableObject {
    @Published private(set) var previewImage: CGImage?
    @Published private(set) var displayState = TrackIRCameraDisplayState()

    let backendLabel = "C + libusb"

    private let mouseController = otir_mac_mouse_controller_create()
    private var pollTask: Task<Void, Never>?
    private var session: OpaquePointer?
    private var pollingConfiguration: TrackIRPollingConfiguration?

    var sourceLabel: String {
        switch displayState.phase {
            case .idle:
                return "Idle"
            case .starting:
                return "Opening"
            case .streaming:
                return "TrackIR 5"
            case .unavailable:
                return "No device"
            case .failed:
                return "Error"
        }
    }

    var frameLabel: String {
        displayState.frameIndex == 0 ? "-" : "#\(displayState.frameIndex)"
    }

    var frameRateLabel: String {
        trackIRFrameRateLabel(for: displayState.frameRate)
    }

    var centroidPairLabel: String {
        trackIRCoordinatePairLabel(x: displayState.centroidX, y: displayState.centroidY)
    }

    var packetTypeLabel: String {
        guard let lastPacketType = displayState.lastPacketType else {
            return "-"
        }

        return String(format: "0x%02X", lastPacketType)
    }

    var phase: TrackIRCameraPhase {
        displayState.phase
    }

    var lastErrorDescription: String? {
        displayState.lastErrorDescription
    }

    var frameIndex: UInt64 {
        displayState.frameIndex
    }

    var frameRate: Double? {
        displayState.frameRate
    }

    var centroidX: Double? {
        displayState.centroidX
    }

    var centroidY: Double? {
        displayState.centroidY
    }

    var lastPacketType: UInt8? {
        displayState.lastPacketType
    }

    func syncStreaming(
        isTrackIREnabled: Bool,
        isVideoEnabled: Bool,
        maximumTrackingFramesPerSecond: Double,
        isWindowVisible: Bool,
        isMouseMovementEnabled: Bool,
        mouseMovementSpeed: Double,
        mouseSmoothing: Int,
        mouseDeadzone: Double,
        isAvoidMouseJumpsEnabled: Bool,
        mouseJumpThresholdPixels: Int,
        minimumBlobAreaPoints: Int,
        isScaledHullContoursEnabled: Bool,
        keepAwakeSeconds: Int,
        mouseTransform: VideoPreviewTransform
    ) {
        let effectiveVideoEnabled = trackIREffectiveVideoEnabled(
            isVideoEnabled: isVideoEnabled,
            isWindowVisible: isWindowVisible
        )
        let shouldPollSnapshots = trackIRShouldPollSnapshots(
            isWindowVisible: isWindowVisible,
            isMouseMovementEnabled: isMouseMovementEnabled,
            isTrackIREnabled: isTrackIREnabled,
            keepAwakeSeconds: keepAwakeSeconds
        )

        if shouldAccessTrackIRHardware(
            isTrackIREnabled: isTrackIREnabled,
            isVideoEnabled: effectiveVideoEnabled,
            environment: ProcessInfo.processInfo.environment
        ) {
            startStreamingIfNeeded(
                isVideoEnabled: effectiveVideoEnabled,
                maximumTrackingFramesPerSecond: maximumTrackingFramesPerSecond,
                shouldPollSnapshots: shouldPollSnapshots,
                shouldPublishUI: isWindowVisible,
                isMouseMovementEnabled: isMouseMovementEnabled,
                mouseMovementSpeed: mouseMovementSpeed,
                mouseSmoothing: mouseSmoothing,
                mouseDeadzone: mouseDeadzone,
                isAvoidMouseJumpsEnabled: isAvoidMouseJumpsEnabled,
                mouseJumpThresholdPixels: mouseJumpThresholdPixels,
                minimumBlobAreaPoints: minimumBlobAreaPoints,
                isScaledHullContoursEnabled: isScaledHullContoursEnabled,
                keepAwakeSeconds: keepAwakeSeconds,
                mouseTransform: mouseTransform
            )
        } else {
            if shouldStreamTrackIRSession(
                isTrackIREnabled: isTrackIREnabled,
                isVideoEnabled: effectiveVideoEnabled
            ), isTrackIRHardwareAccessDisabledForHostEnvironment(
                environment: ProcessInfo.processInfo.environment
            ) {
                trackIRLogger.info("Skipping TrackIR hardware access in host test or preview mode")
            }
            stopStreaming(clearPreview: true, waitForShutdown: false)
        }
    }

    func refresh(
        isTrackIREnabled: Bool,
        isVideoEnabled: Bool,
        maximumTrackingFramesPerSecond: Double,
        isWindowVisible: Bool,
        isMouseMovementEnabled: Bool,
        mouseMovementSpeed: Double,
        mouseSmoothing: Int,
        mouseDeadzone: Double,
        isAvoidMouseJumpsEnabled: Bool,
        mouseJumpThresholdPixels: Int,
        minimumBlobAreaPoints: Int,
        isScaledHullContoursEnabled: Bool,
        keepAwakeSeconds: Int,
        mouseTransform: VideoPreviewTransform
    ) {
        let shouldPollSnapshots = trackIRShouldPollSnapshots(
            isWindowVisible: isWindowVisible,
            isMouseMovementEnabled: isMouseMovementEnabled,
            isTrackIREnabled: isTrackIREnabled,
            keepAwakeSeconds: keepAwakeSeconds
        )
        let shouldRestart = shouldAccessTrackIRHardware(
            isTrackIREnabled: isTrackIREnabled,
            isVideoEnabled: isVideoEnabled,
            environment: ProcessInfo.processInfo.environment
        )

        trackIRLogger.info(
            "Refresh requested. trackIREnabled=\(isTrackIREnabled, privacy: .public) videoEnabled=\(isVideoEnabled, privacy: .public) shouldRestart=\(shouldRestart, privacy: .public)"
        )

        stopStreaming(clearPreview: true, waitForShutdown: true)

        guard shouldRestart else {
            return
        }

        startStreamingIfNeeded(
            isVideoEnabled: isVideoEnabled,
            maximumTrackingFramesPerSecond: maximumTrackingFramesPerSecond,
            shouldPollSnapshots: shouldPollSnapshots,
            shouldPublishUI: isWindowVisible,
            isMouseMovementEnabled: isMouseMovementEnabled,
            mouseMovementSpeed: mouseMovementSpeed,
            mouseSmoothing: mouseSmoothing,
            mouseDeadzone: mouseDeadzone,
            isAvoidMouseJumpsEnabled: isAvoidMouseJumpsEnabled,
            mouseJumpThresholdPixels: mouseJumpThresholdPixels,
            minimumBlobAreaPoints: minimumBlobAreaPoints,
            isScaledHullContoursEnabled: isScaledHullContoursEnabled,
            keepAwakeSeconds: keepAwakeSeconds,
            mouseTransform: mouseTransform
        )
    }

    func shutdown() {
        stopStreaming(clearPreview: true, waitForShutdown: false)
    }

    func shutdownAndWait() {
        stopStreaming(clearPreview: true, waitForShutdown: true)
    }

    private func startStreamingIfNeeded(
        isVideoEnabled: Bool,
        maximumTrackingFramesPerSecond: Double,
        shouldPollSnapshots: Bool,
        shouldPublishUI: Bool,
        isMouseMovementEnabled: Bool,
        mouseMovementSpeed: Double,
        mouseSmoothing: Int,
        mouseDeadzone: Double,
        isAvoidMouseJumpsEnabled: Bool,
        mouseJumpThresholdPixels: Int,
        minimumBlobAreaPoints: Int,
        isScaledHullContoursEnabled: Bool,
        keepAwakeSeconds: Int,
        mouseTransform: VideoPreviewTransform
    ) {
        guard let session = ensureSession() else {
            displayState = TrackIRCameraDisplayState(
                phase: .failed,
                lastErrorDescription: "Failed to create TrackIR session."
            )
            return
        }

        otir_trackir_session_set_maximum_tracking_frames_per_second(
            session,
            maximumTrackingFramesPerSecond
        )
        otir_trackir_session_set_video_enabled(session, isVideoEnabled)
        otir_trackir_session_set_minimum_blob_area_points(session, Int32(minimumBlobAreaPoints))
        otir_trackir_session_set_scaled_hull_enabled(session, isScaledHullContoursEnabled)
        otir_mac_mouse_controller_prepare_post_event_access(
            mouseController,
            trackIRShouldRequestMouseEventAccess(
                isMouseMovementEnabled: isMouseMovementEnabled,
                keepAwakeSeconds: keepAwakeSeconds
            )
        )

        let startStatus = otir_trackir_session_start(session)
        guard startStatus == OTIR_STATUS_OK else {
            displayState = TrackIRCameraDisplayState(
                phase: .failed,
                lastErrorDescription: trackIRFailureMessage(
                    for: startStatus,
                    operation: "Start session"
                )
            )
            return
        }

        if pollTask == nil {
            displayState = TrackIRCameraDisplayState(phase: .starting)
            previewImage = nil
        }

        if !isVideoEnabled {
            previewImage = nil
        }

        if !shouldPollSnapshots {
            pollTask?.cancel()
            pollTask = nil
            pollingConfiguration = nil
            otir_mac_mouse_controller_reset(mouseController)
            previewImage = nil
            return
        }

        let configuration = TrackIRPollingConfiguration(
            isTrackIREnabled: true,
            isVideoEnabled: isVideoEnabled,
            isMouseMovementEnabled: isMouseMovementEnabled,
            mouseMovementSpeed: mouseMovementSpeed,
            mouseSmoothing: mouseSmoothing,
            mouseDeadzone: mouseDeadzone,
            isAvoidMouseJumpsEnabled: isAvoidMouseJumpsEnabled,
            mouseJumpThresholdPixels: mouseJumpThresholdPixels,
            minimumBlobAreaPoints: minimumBlobAreaPoints,
            isScaledHullContoursEnabled: isScaledHullContoursEnabled,
            keepAwakeSeconds: keepAwakeSeconds,
            mouseTransform: mouseTransform,
            shouldPublishUI: shouldPublishUI,
            maximumPreviewFramesPerSecond: 30.0,
            maximumTelemetryFramesPerSecond: 10.0,
            pollInterval: trackIRPollingInterval(
                isVideoEnabled: isVideoEnabled,
                isMouseMovementEnabled: isMouseMovementEnabled,
                maximumTrackingFramesPerSecond: maximumTrackingFramesPerSecond
            )
        )

        if pollTask == nil || pollingConfiguration != configuration {
            pollingConfiguration = configuration
            otir_mac_mouse_controller_reset(mouseController)
            startPolling(
                session: session,
                mouseController: mouseController,
                configuration: configuration
            )
        }
    }

    private func stopStreaming(clearPreview: Bool, waitForShutdown: Bool) {
        pollTask?.cancel()
        pollTask = nil

        if let session {
            trackIRLogger.info("Stopping TrackIR session")
            otir_trackir_session_stop(session, waitForShutdown)
        }

        otir_mac_mouse_controller_reset(mouseController)
        displayState = TrackIRCameraDisplayState()
        pollingConfiguration = nil

        if clearPreview {
            previewImage = nil
        }
    }

    private func ensureSession() -> OpaquePointer? {
        if session == nil {
            session = otir_trackir_session_create()
        }

        return session
    }

    private func startPolling(
        session: OpaquePointer,
        mouseController: OpaquePointer?,
        configuration: TrackIRPollingConfiguration
    ) {
        pollTask?.cancel()

        pollTask = Task.detached(priority: .utility) { [weak self] in
            var lastPreviewFrameGeneration: UInt64 = 0
            var lastPreviewUpdateTime: TimeInterval?
            var lastDisplayUpdateTime: TimeInterval?
            var lastPublishedDisplayState: TrackIRCameraDisplayState?
            var hasPublishedPreviewImage = false
            var lastMouseMovementTime = ProcessInfo.processInfo.systemUptime

            while !Task.isCancelled {
                var snapshot = otir_trackir_session_snapshot()
                var latestPreviewImage: CGImage?

                otir_trackir_session_copy_snapshot(session, &snapshot)

                let currentTime = ProcessInfo.processInfo.systemUptime
                let elapsedPreviewTime = lastPreviewUpdateTime.map { currentTime - $0 }

                if trackIRShouldUpdatePreviewFrame(
                    isVideoEnabled: configuration.isVideoEnabled,
                    hasPreviewFrame: snapshot.has_preview_frame,
                    previewFrameGeneration: snapshot.preview_frame_generation,
                    lastPreviewFrameGeneration: lastPreviewFrameGeneration,
                    elapsedTimeSinceLastPreview: elapsedPreviewTime,
                    maximumPreviewFramesPerSecond: configuration.maximumPreviewFramesPerSecond
                ) {
                    var frameBytes = [UInt8](
                        repeating: 0,
                        count: Int(OTIR_TRACKIR_SESSION_FRAME_BYTES)
                    )
                    var frameGeneration: UInt64 = 0

                    let didCopyPreviewFrame = frameBytes.withUnsafeMutableBufferPointer { buffer in
                        otir_trackir_session_copy_preview_frame(
                            session,
                            buffer.baseAddress,
                            buffer.count,
                            &frameGeneration
                        )
                    }

                    if didCopyPreviewFrame {
                        latestPreviewImage = trackIRPreviewImage(
                            frameBytes: frameBytes,
                            width: Int(snapshot.preview_width),
                            height: Int(snapshot.preview_height)
                        )
                        lastPreviewFrameGeneration = frameGeneration
                        lastPreviewUpdateTime = currentTime
                    }
                }

                let snapshotCopy = snapshot
                let previewImageCopy = latestPreviewImage
                let nextDisplayState = trackIRDisplayState(snapshot: snapshotCopy)
                let elapsedDisplayTime = lastDisplayUpdateTime.map { currentTime - $0 }
                let shouldPublishDisplayState = trackIRShouldPublishDisplayState(
                    shouldPublishUI: configuration.shouldPublishUI,
                    currentDisplayState: nextDisplayState,
                    lastPublishedDisplayState: lastPublishedDisplayState,
                    elapsedTimeSinceLastDisplay: elapsedDisplayTime,
                    maximumDisplayFramesPerSecond: configuration.maximumTelemetryFramesPerSecond
                )
                let shouldClearPreviewImage = configuration.shouldPublishUI &&
                    hasPublishedPreviewImage &&
                    (!configuration.isVideoEnabled ||
                        !snapshotCopy.has_preview_frame ||
                        snapshotCopy.phase != OTIR_TRACKIR_SESSION_PHASE_STREAMING)
                let didMoveMouse = trackIRApplyMouseMovement(
                    controller: mouseController,
                    snapshot: snapshotCopy,
                    configuration: configuration
                )
                if didMoveMouse {
                    lastMouseMovementTime = currentTime
                } else if trackIRShouldFireKeepAwake(
                    isTrackIREnabled: configuration.isTrackIREnabled,
                    isMouseMovementEnabled: configuration.isMouseMovementEnabled,
                    keepAwakeSeconds: configuration.keepAwakeSeconds,
                    timeSinceLastMouseMovement: currentTime - lastMouseMovementTime
                ), otir_mac_mouse_controller_nudge(mouseController) {
                    lastMouseMovementTime = currentTime
                }

                if shouldPublishDisplayState || previewImageCopy != nil || shouldClearPreviewImage {
                    let cameraController = self
                    await MainActor.run {
                        cameraController?.apply(
                            displayState: nextDisplayState,
                            previewImage: previewImageCopy,
                            shouldUpdateDisplayState: shouldPublishDisplayState,
                            shouldClearPreviewImage: shouldClearPreviewImage
                        )
                    }
                }

                if shouldPublishDisplayState {
                    lastPublishedDisplayState = nextDisplayState
                    lastDisplayUpdateTime = currentTime
                }
                if previewImageCopy != nil {
                    hasPublishedPreviewImage = true
                } else if shouldClearPreviewImage {
                    hasPublishedPreviewImage = false
                }

                let nanoseconds = UInt64(configuration.pollInterval * 1_000_000_000)
                try? await Task.sleep(nanoseconds: nanoseconds)
            }
        }
    }

    private func apply(
        displayState: TrackIRCameraDisplayState,
        previewImage: CGImage?,
        shouldUpdateDisplayState: Bool,
        shouldClearPreviewImage: Bool
    ) {
        if shouldUpdateDisplayState, self.displayState != displayState {
            self.displayState = displayState
        }

        if let previewImage {
            self.previewImage = previewImage
        } else if shouldClearPreviewImage {
            self.previewImage = nil
        }
    }

    deinit {
        pollTask?.cancel()

        if let session {
            otir_trackir_session_stop(session, true)
            otir_trackir_session_destroy(session)
        }

        otir_mac_mouse_controller_destroy(mouseController)
    }
}

private let trackIRLogger = Logger(
    subsystem: Bundle.main.bundleIdentifier ?? "philsapps.OpenTrackIR",
    category: "TrackIRCamera"
)

private struct TrackIRPollingConfiguration: Equatable {
    let isTrackIREnabled: Bool
    let isVideoEnabled: Bool
    let isMouseMovementEnabled: Bool
    let mouseMovementSpeed: Double
    let mouseSmoothing: Int
    let mouseDeadzone: Double
    let isAvoidMouseJumpsEnabled: Bool
    let mouseJumpThresholdPixels: Int
    let minimumBlobAreaPoints: Int
    let isScaledHullContoursEnabled: Bool
    let keepAwakeSeconds: Int
    let mouseTransform: VideoPreviewTransform
    let shouldPublishUI: Bool
    let maximumPreviewFramesPerSecond: Double
    let maximumTelemetryFramesPerSecond: Double
    let pollInterval: TimeInterval
}

nonisolated func shouldStreamTrackIRSession(isTrackIREnabled: Bool, isVideoEnabled: Bool) -> Bool {
    isTrackIREnabled
}

nonisolated func trackIREffectiveVideoEnabled(isVideoEnabled: Bool, isWindowVisible: Bool) -> Bool {
    isVideoEnabled && isWindowVisible
}

nonisolated func isRunningInXcodePreview(environment: [String: String]) -> Bool {
    environment["XCODE_RUNNING_FOR_PREVIEWS"] == "1" ||
        environment["XCODE_RUNNING_FOR_PLAYGROUNDS"] == "1"
}

nonisolated func isTrackIRHardwareAccessDisabledForHostEnvironment(
    environment: [String: String]
) -> Bool {
    isRunningInXcodePreview(environment: environment) ||
        environment["OTIR_DISABLE_TRACKIR_HARDWARE"] == "1"
}

nonisolated func shouldAccessTrackIRHardware(
    isTrackIREnabled: Bool,
    isVideoEnabled: Bool,
    environment: [String: String]
) -> Bool {
    shouldStreamTrackIRSession(
        isTrackIREnabled: isTrackIREnabled,
        isVideoEnabled: isVideoEnabled
    ) && !isTrackIRHardwareAccessDisabledForHostEnvironment(environment: environment)
}

nonisolated func trackIRShouldPollSnapshots(
    isWindowVisible: Bool,
    isMouseMovementEnabled: Bool,
    isTrackIREnabled: Bool,
    keepAwakeSeconds: Int
) -> Bool {
    isWindowVisible || isMouseMovementEnabled || (isTrackIREnabled && keepAwakeSeconds > 0)
}

nonisolated func trackIRShouldRequestMouseEventAccess(
    isMouseMovementEnabled: Bool,
    keepAwakeSeconds: Int
) -> Bool {
    isMouseMovementEnabled || keepAwakeSeconds > 0
}

nonisolated func trackIRShouldFireKeepAwake(
    isTrackIREnabled: Bool,
    isMouseMovementEnabled: Bool,
    keepAwakeSeconds: Int,
    timeSinceLastMouseMovement: TimeInterval
) -> Bool {
    isTrackIREnabled &&
        !isMouseMovementEnabled &&
        keepAwakeSeconds > 0 &&
        timeSinceLastMouseMovement >= Double(keepAwakeSeconds)
}

nonisolated func trackIRPollingInterval(
    isVideoEnabled: Bool,
    isMouseMovementEnabled: Bool,
    maximumTrackingFramesPerSecond: Double
) -> TimeInterval {
    guard isVideoEnabled || isMouseMovementEnabled else {
        return 0.2
    }

    let framesPerSecond = maximumTrackingFramesPerSecond > 0
        ? maximumTrackingFramesPerSecond
        : 60.0

    return 1.0 / framesPerSecond
}

nonisolated func trackIRShouldUpdatePreviewFrame(
    isVideoEnabled: Bool,
    hasPreviewFrame: Bool,
    previewFrameGeneration: UInt64,
    lastPreviewFrameGeneration: UInt64,
    elapsedTimeSinceLastPreview: TimeInterval?,
    maximumPreviewFramesPerSecond: Double
) -> Bool {
    guard isVideoEnabled, hasPreviewFrame else {
        return false
    }

    guard previewFrameGeneration != lastPreviewFrameGeneration else {
        return false
    }

    guard maximumPreviewFramesPerSecond > 0 else {
        return true
    }

    guard let elapsedTimeSinceLastPreview else {
        return true
    }

    return elapsedTimeSinceLastPreview >= (1.0 / maximumPreviewFramesPerSecond)
}

nonisolated func trackIRCameraPhase(sessionPhase: otir_trackir_session_phase) -> TrackIRCameraPhase {
    switch sessionPhase {
        case OTIR_TRACKIR_SESSION_PHASE_STARTING:
            return .starting
        case OTIR_TRACKIR_SESSION_PHASE_STREAMING:
            return .streaming
        case OTIR_TRACKIR_SESSION_PHASE_UNAVAILABLE:
            return .unavailable
        case OTIR_TRACKIR_SESSION_PHASE_FAILED:
            return .failed
        default:
            return .idle
    }
}

nonisolated func trackIRSessionErrorDescription(snapshot: otir_trackir_session_snapshot) -> String? {
    guard snapshot.has_error_message else {
        return nil
    }

    let bytes = withUnsafeBytes(of: snapshot.error_message) { buffer in
        Array(buffer.prefix { $0 != 0 })
    }

    guard !bytes.isEmpty else {
        return nil
    }

    return String(bytes: bytes, encoding: .utf8)
}

nonisolated func trackIRDisplayState(
    snapshot: otir_trackir_session_snapshot
) -> TrackIRCameraDisplayState {
    TrackIRCameraDisplayState(
        phase: trackIRCameraPhase(sessionPhase: snapshot.phase),
        lastErrorDescription: trackIRSessionErrorDescription(snapshot: snapshot),
        frameIndex: snapshot.frame_index,
        frameRate: snapshot.has_frame_rate ? snapshot.frame_rate : nil,
        centroidX: snapshot.has_centroid ? snapshot.centroid_x : nil,
        centroidY: snapshot.has_centroid ? snapshot.centroid_y : nil,
        lastPacketType: snapshot.has_packet_type ? snapshot.packet_type : nil
    )
}

nonisolated func trackIRShouldPublishDisplayState(
    shouldPublishUI: Bool,
    currentDisplayState: TrackIRCameraDisplayState,
    lastPublishedDisplayState: TrackIRCameraDisplayState?,
    elapsedTimeSinceLastDisplay: TimeInterval?,
    maximumDisplayFramesPerSecond: Double
) -> Bool {
    guard shouldPublishUI else {
        return false
    }

    if lastPublishedDisplayState?.phase != currentDisplayState.phase ||
        lastPublishedDisplayState?.lastErrorDescription != currentDisplayState.lastErrorDescription {
        return true
    }

    guard maximumDisplayFramesPerSecond > 0 else {
        return true
    }

    guard let elapsedTimeSinceLastDisplay else {
        return true
    }

    return elapsedTimeSinceLastDisplay >= (1.0 / maximumDisplayFramesPerSecond)
}

nonisolated func trackIRFrameRateLabel(for frameRate: Double?) -> String {
    guard let frameRate else {
        return "-"
    }

    return "\(frameRate.formatted(.number.precision(.fractionLength(1)))) fps"
}

nonisolated func trackIRCoordinateLabel(for coordinate: Double?) -> String {
    guard let coordinate else {
        return "-"
    }

    return String(Int(coordinate))
}

nonisolated func trackIRCoordinatePairLabel(x: Double?, y: Double?) -> String {
    let xLabel = trackIRCoordinateLabel(for: x)
    let yLabel = trackIRCoordinateLabel(for: y)

    guard xLabel != "-", yLabel != "-" else {
        return "-"
    }

    return "\(xLabel), \(yLabel)"
}

nonisolated func trackIRMouseTransform(
    _ transform: VideoPreviewTransform
) -> otir_trackir_mouse_transform {
    otir_trackir_mouse_transform(
        scale_x: Double(transform.scaleX),
        scale_y: Double(transform.scaleY),
        rotation_degrees: transform.rotationDegrees
    )
}

private nonisolated func trackIRApplyMouseMovement(
    controller: OpaquePointer?,
    snapshot: otir_trackir_session_snapshot,
    configuration: TrackIRPollingConfiguration
) -> Bool {
    otir_mac_mouse_controller_update(
        controller,
        snapshot.has_centroid,
        snapshot.centroid_x,
        snapshot.centroid_y,
        otir_trackir_mouse_tracker_config(
            is_movement_enabled: configuration.isMouseMovementEnabled &&
                snapshot.phase == OTIR_TRACKIR_SESSION_PHASE_STREAMING,
            speed: configuration.mouseMovementSpeed,
            smoothing: Double(configuration.mouseSmoothing),
            deadzone: configuration.mouseDeadzone,
            avoid_mouse_jumps: configuration.isAvoidMouseJumpsEnabled,
            jump_threshold_pixels: Double(configuration.mouseJumpThresholdPixels),
            transform: trackIRMouseTransform(configuration.mouseTransform)
        )
    )
}

nonisolated func trackIRPreviewImage(frameBytes: [UInt8], width: Int, height: Int) -> CGImage? {
    guard width > 0, height > 0, frameBytes.count == width * height else {
        return nil
    }

    let frameData = Data(frameBytes)
    guard let dataProvider = CGDataProvider(data: frameData as CFData) else {
        return nil
    }

    return CGImage(
        width: width,
        height: height,
        bitsPerComponent: 8,
        bitsPerPixel: 8,
        bytesPerRow: width,
        space: CGColorSpaceCreateDeviceGray(),
        bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.none.rawValue),
        provider: dataProvider,
        decode: nil,
        shouldInterpolate: false,
        intent: .defaultIntent
    )
}

nonisolated func trackIRFailureMessage(for status: otir_status, operation: String) -> String {
    if status == OTIR_STATUS_NOT_FOUND {
        return "TrackIR not found. Connect the device and try again."
    }

    if status == OTIR_STATUS_IO, operation == "Open" {
        return "Open failed: io. TrackIR may be busy in another app. Quit other TrackIR tools, then Refresh."
    }

    guard let statusDescription = otir_status_string(status) else {
        return "\(operation) failed."
    }

    return "\(operation) failed: \(String(cString: statusDescription))."
}

nonisolated func trackIRPreviewMessage(
    isTrackIREnabled: Bool,
    isVideoEnabled: Bool,
    phase: TrackIRCameraPhase,
    errorDescription: String?
) -> String {
    if !isVideoEnabled {
        return "Turn video on to show the camera."
    }

    if !isTrackIREnabled {
        return "Turn TrackIR on to start the camera."
    }

    switch phase {
        case .idle, .starting:
            return "Opening the TrackIR camera."
        case .streaming:
            return "Live camera feed from the shared C library."
        case .unavailable, .failed:
            return errorDescription ?? "TrackIR camera unavailable."
    }
}
