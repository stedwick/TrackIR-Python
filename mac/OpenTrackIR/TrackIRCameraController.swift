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

@MainActor
final class TrackIRCameraController: ObservableObject {
    @Published private(set) var previewImage: CGImage?
    @Published private(set) var phase: TrackIRCameraPhase = .idle
    @Published private(set) var lastErrorDescription: String?
    @Published private(set) var frameIndex: UInt64 = 0
    @Published private(set) var frameRate: Double?
    @Published private(set) var centroidX: Double?
    @Published private(set) var centroidY: Double?
    @Published private(set) var lastPacketType: UInt8?

    let backendLabel = "C + libusb"

    private let mouseController = TrackIRMouseController()
    private var pollTask: Task<Void, Never>?
    private var session: OpaquePointer?
    private var pollingConfiguration: TrackIRPollingConfiguration?

    var sourceLabel: String {
        switch phase {
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
        frameIndex == 0 ? "-" : "#\(frameIndex)"
    }

    var frameRateLabel: String {
        trackIRFrameRateLabel(for: frameRate)
    }

    var centroidPairLabel: String {
        trackIRCoordinatePairLabel(x: centroidX, y: centroidY)
    }

    var packetTypeLabel: String {
        guard let lastPacketType else {
            return "-"
        }

        return String(format: "0x%02X", lastPacketType)
    }

    func syncStreaming(
        isTrackIREnabled: Bool,
        isVideoEnabled: Bool,
        maximumTrackingFramesPerSecond: Double,
        isWindowVisible: Bool,
        isMouseMovementEnabled: Bool,
        mouseMovementSpeed: Double,
        mouseTransform: VideoPreviewTransform
    ) {
        let effectiveVideoEnabled = trackIREffectiveVideoEnabled(
            isVideoEnabled: isVideoEnabled,
            isWindowVisible: isWindowVisible
        )
        let shouldPollSnapshots = trackIRShouldPollSnapshots(
            isWindowVisible: isWindowVisible,
            isMouseMovementEnabled: isMouseMovementEnabled
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
                isMouseMovementEnabled: isMouseMovementEnabled,
                mouseMovementSpeed: mouseMovementSpeed,
                mouseTransform: mouseTransform
            )
        } else {
            if shouldStreamTrackIRSession(
                isTrackIREnabled: isTrackIREnabled,
                isVideoEnabled: effectiveVideoEnabled
            ), isRunningInXcodePreview(environment: ProcessInfo.processInfo.environment) {
                trackIRLogger.info("Skipping TrackIR hardware access in Xcode preview mode")
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
        mouseTransform: VideoPreviewTransform
    ) {
        let shouldPollSnapshots = trackIRShouldPollSnapshots(
            isWindowVisible: isWindowVisible,
            isMouseMovementEnabled: isMouseMovementEnabled
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
            isMouseMovementEnabled: isMouseMovementEnabled,
            mouseMovementSpeed: mouseMovementSpeed,
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
        isMouseMovementEnabled: Bool,
        mouseMovementSpeed: Double,
        mouseTransform: VideoPreviewTransform
    ) {
        guard let session = ensureSession() else {
            phase = .failed
            lastErrorDescription = "Failed to create TrackIR session."
            return
        }

        otir_trackir_session_set_maximum_tracking_frames_per_second(
            session,
            maximumTrackingFramesPerSecond
        )
        otir_trackir_session_set_video_enabled(session, isVideoEnabled)

        let startStatus = otir_trackir_session_start(session)
        guard startStatus == OTIR_STATUS_OK else {
            phase = .failed
            lastErrorDescription = trackIRFailureMessage(for: startStatus, operation: "Start session")
            return
        }

        if pollTask == nil {
            phase = .starting
            lastErrorDescription = nil
            frameIndex = 0
            frameRate = nil
            centroidX = nil
            centroidY = nil
            lastPacketType = nil
            previewImage = nil
        }

        if !isVideoEnabled {
            previewImage = nil
        }

        if !shouldPollSnapshots {
            pollTask?.cancel()
            pollTask = nil
            pollingConfiguration = nil
            mouseController.reset()
            previewImage = nil
            return
        }

        let configuration = TrackIRPollingConfiguration(
            isVideoEnabled: isVideoEnabled,
            isMouseMovementEnabled: isMouseMovementEnabled,
            mouseMovementSpeed: mouseMovementSpeed,
            mouseTransform: mouseTransform,
            maximumPreviewFramesPerSecond: 30.0,
            pollInterval: trackIRPollingInterval(
                isVideoEnabled: isVideoEnabled,
                isMouseMovementEnabled: isMouseMovementEnabled,
                maximumTrackingFramesPerSecond: maximumTrackingFramesPerSecond
            )
        )

        if pollTask == nil || pollingConfiguration != configuration {
            pollingConfiguration = configuration
            mouseController.reset()
            startPolling(session: session, configuration: configuration)
        }
    }

    private func stopStreaming(clearPreview: Bool, waitForShutdown: Bool) {
        pollTask?.cancel()
        pollTask = nil

        if let session {
            trackIRLogger.info("Stopping TrackIR session")
            otir_trackir_session_stop(session, waitForShutdown)
        }

        mouseController.reset()
        phase = .idle
        lastErrorDescription = nil
        frameIndex = 0
        frameRate = nil
        centroidX = nil
        centroidY = nil
        lastPacketType = nil
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

    private func startPolling(session: OpaquePointer, configuration: TrackIRPollingConfiguration) {
        pollTask?.cancel()

        pollTask = Task.detached(priority: .utility) { [weak self] in
            var lastPreviewFrameGeneration: UInt64 = 0
            var lastPreviewUpdateTime: TimeInterval?

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

                await MainActor.run { [weak self, snapshotCopy, previewImageCopy] in
                    self?.apply(
                        snapshotCopy,
                        previewImage: previewImageCopy,
                        configuration: configuration
                    )
                }

                let nanoseconds = UInt64(configuration.pollInterval * 1_000_000_000)
                try? await Task.sleep(nanoseconds: nanoseconds)
            }
        }
    }

    private func apply(
        _ snapshot: otir_trackir_session_snapshot,
        previewImage: CGImage?,
        configuration: TrackIRPollingConfiguration
    ) {
        phase = trackIRCameraPhase(sessionPhase: snapshot.phase)
        lastErrorDescription = trackIRSessionErrorDescription(snapshot: snapshot)
        frameIndex = snapshot.frame_index
        frameRate = snapshot.has_frame_rate ? snapshot.frame_rate : nil
        centroidX = snapshot.has_centroid ? snapshot.centroid_x : nil
        centroidY = snapshot.has_centroid ? snapshot.centroid_y : nil
        lastPacketType = snapshot.has_packet_type ? snapshot.packet_type : nil

        mouseController.update(
            centroidX: centroidX,
            centroidY: centroidY,
            isMovementEnabled: configuration.isMouseMovementEnabled && phase == .streaming,
            speed: configuration.mouseMovementSpeed,
            transform: configuration.mouseTransform
        )

        if let previewImage {
            self.previewImage = previewImage
        } else if !configuration.isVideoEnabled || !snapshot.has_preview_frame || phase != .streaming {
            self.previewImage = nil
        }
    }

    deinit {
        pollTask?.cancel()

        if let session {
            otir_trackir_session_stop(session, true)
            otir_trackir_session_destroy(session)
        }
    }
}

private let trackIRLogger = Logger(
    subsystem: Bundle.main.bundleIdentifier ?? "philsapps.OpenTrackIR",
    category: "TrackIRCamera"
)

private struct TrackIRPollingConfiguration: Equatable {
    let isVideoEnabled: Bool
    let isMouseMovementEnabled: Bool
    let mouseMovementSpeed: Double
    let mouseTransform: VideoPreviewTransform
    let maximumPreviewFramesPerSecond: Double
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

nonisolated func shouldAccessTrackIRHardware(
    isTrackIREnabled: Bool,
    isVideoEnabled: Bool,
    environment: [String: String]
) -> Bool {
    shouldStreamTrackIRSession(
        isTrackIREnabled: isTrackIREnabled,
        isVideoEnabled: isVideoEnabled
    ) && !isRunningInXcodePreview(environment: environment)
}

nonisolated func trackIRShouldPollSnapshots(
    isWindowVisible: Bool,
    isMouseMovementEnabled: Bool
) -> Bool {
    isWindowVisible || isMouseMovementEnabled
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
