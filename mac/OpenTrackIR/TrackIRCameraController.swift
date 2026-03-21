import Combine
import CoreGraphics
import Foundation
import OSLog
import SwiftUI

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

    private var activeSessionToken = 0
    private var restartTask: Task<Void, Never>?
    private var runner: TrackIRStreamRunner?

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

    func syncStreaming(isTrackIREnabled: Bool, isVideoEnabled: Bool) {
        restartTask?.cancel()
        restartTask = nil

        if shouldAccessTrackIRHardware(
            isTrackIREnabled: isTrackIREnabled,
            isVideoEnabled: isVideoEnabled,
            environment: ProcessInfo.processInfo.environment
        ) {
            startStreamingIfNeeded(isVideoEnabled: isVideoEnabled)
        } else {
            if shouldStreamTrackIRSession(isTrackIREnabled: isTrackIREnabled, isVideoEnabled: isVideoEnabled),
               isRunningInXcodePreview(environment: ProcessInfo.processInfo.environment) {
                trackIRLogger.info("Skipping TrackIR hardware access in Xcode preview mode")
            }
            stopStreaming(clearPreview: true, waitForRunnerExit: false)
        }
    }

    func refresh(isTrackIREnabled: Bool, isVideoEnabled: Bool) {
        let shouldRestart = shouldAccessTrackIRHardware(
            isTrackIREnabled: isTrackIREnabled,
            isVideoEnabled: isVideoEnabled,
            environment: ProcessInfo.processInfo.environment
        )

        trackIRLogger.info(
            "Refresh requested. trackIREnabled=\(isTrackIREnabled, privacy: .public) videoEnabled=\(isVideoEnabled, privacy: .public) shouldRestart=\(shouldRestart, privacy: .public)"
        )

        restartTask?.cancel()
        restartTask = nil
        stopStreaming(clearPreview: true, waitForRunnerExit: false)

        guard shouldRestart else {
            return
        }

        phase = .starting
        lastErrorDescription = nil

        restartTask = Task { @MainActor [weak self] in
            try? await Task.sleep(for: .milliseconds(125))
            guard let self else {
                return
            }

            self.restartTask = nil
            self.startStreamingIfNeeded(isVideoEnabled: isVideoEnabled)
        }
    }

    func shutdown() {
        restartTask?.cancel()
        restartTask = nil
        stopStreaming(clearPreview: true, waitForRunnerExit: false)
    }

    func shutdownAndWait() {
        restartTask?.cancel()
        restartTask = nil
        stopStreaming(clearPreview: true, waitForRunnerExit: true)
    }

    private func startStreamingIfNeeded(isVideoEnabled: Bool) {
        restartTask?.cancel()
        restartTask = nil

        guard runner == nil else {
            runner?.setVideoPreviewEnabled(isVideoEnabled)
            if !isVideoEnabled {
                previewImage = nil
            }
            return
        }

        activeSessionToken += 1
        let sessionToken = activeSessionToken

        trackIRLogger.info("Starting TrackIR session \(sessionToken, privacy: .public)")

        phase = .starting
        lastErrorDescription = nil
        frameIndex = 0
        frameRate = nil
        centroidX = nil
        centroidY = nil
        lastPacketType = nil
        previewImage = nil

        let runner = TrackIRStreamRunner(
            sessionToken: sessionToken,
            isVideoPreviewEnabled: isVideoEnabled
        ) { [weak self] event in
            Task { @MainActor [weak self] in
                self?.apply(event)
            }
        }

        self.runner = runner
        runner.start()
    }

    private func stopStreaming(clearPreview: Bool, waitForRunnerExit: Bool) {
        let activeRunner = runner

        if activeRunner != nil {
            trackIRLogger.info("Stopping TrackIR session \(self.activeSessionToken, privacy: .public)")
        }

        activeSessionToken += 1
        runner = nil
        activeRunner?.requestStop()

        if waitForRunnerExit, let activeRunner {
            let waitMilliseconds = trackIRShutdownWaitMilliseconds(
                readTimeoutMilliseconds: TrackIRStreamRunner.readTimeoutMilliseconds
            )
            let didStopInTime = activeRunner.waitUntilStopped(
                timeout: .now() + .milliseconds(waitMilliseconds)
            )

            if !didStopInTime {
                trackIRLogger.error(
                    "TrackIR session shutdown timed out after \(waitMilliseconds, privacy: .public) ms"
                )
            }
        }

        phase = .idle
        lastErrorDescription = nil
        frameIndex = 0
        frameRate = nil
        centroidX = nil
        centroidY = nil
        lastPacketType = nil

        if clearPreview {
            previewImage = nil
        }
    }

    private func apply(_ event: TrackIRCameraEvent) {
        guard event.sessionToken == activeSessionToken else {
            return
        }

        switch event.kind {
            case .didStartStreaming:
                trackIRLogger.info("TrackIR session \(event.sessionToken, privacy: .public) is streaming")
                phase = .streaming
                lastErrorDescription = nil

            case let .didRenderFrame(image, frameIndex, frameRate, centroidX, centroidY, packetType):
                phase = .streaming
                previewImage = image
                self.frameIndex = frameIndex
                self.frameRate = frameRate
                self.centroidX = centroidX
                self.centroidY = centroidY
                lastPacketType = packetType

            case let .didUpdateTelemetry(frameIndex, frameRate, centroidX, centroidY, packetType):
                phase = .streaming
                self.frameIndex = frameIndex
                self.frameRate = frameRate
                self.centroidX = centroidX
                self.centroidY = centroidY
                lastPacketType = packetType

            case let .didBecomeUnavailable(message):
                trackIRLogger.notice("TrackIR unavailable for session \(event.sessionToken, privacy: .public): \(message, privacy: .public)")
                runner = nil
                previewImage = nil
                phase = .unavailable
                lastErrorDescription = message
                frameIndex = 0
                frameRate = nil
                centroidX = nil
                centroidY = nil
                lastPacketType = nil

            case let .didFail(message):
                trackIRLogger.error("TrackIR failure for session \(event.sessionToken, privacy: .public): \(message, privacy: .public)")
                runner = nil
                previewImage = nil
                phase = .failed
                lastErrorDescription = message
                frameIndex = 0
                frameRate = nil
                centroidX = nil
                centroidY = nil
                lastPacketType = nil
        }
    }
}

private let trackIRLogger = Logger(
    subsystem: Bundle.main.bundleIdentifier ?? "philsapps.OpenTrackIR",
    category: "TrackIRCamera"
)

private struct TrackIRCameraEvent: @unchecked Sendable {
    let sessionToken: Int
    let kind: TrackIRCameraEventKind
}

private enum TrackIRCameraEventKind: @unchecked Sendable {
    case didStartStreaming
    case didRenderFrame(CGImage, UInt64, Double?, Double?, Double?, UInt8)
    case didUpdateTelemetry(UInt64, Double?, Double?, Double?, UInt8)
    case didBecomeUnavailable(String)
    case didFail(String)
}

private final class TrackIRStreamRunner: @unchecked Sendable {
    static let readTimeoutMilliseconds = 50

    private let sessionToken: Int
    private let onEvent: @Sendable (TrackIRCameraEvent) -> Void
    private let lock = NSLock()
    private let stoppedSemaphore = DispatchSemaphore(value: 0)

    private var isStopRequested = false
    private var isVideoPreviewEnabled: Bool

    init(
        sessionToken: Int,
        isVideoPreviewEnabled: Bool,
        onEvent: @escaping @Sendable (TrackIRCameraEvent) -> Void
    ) {
        self.sessionToken = sessionToken
        self.isVideoPreviewEnabled = isVideoPreviewEnabled
        self.onEvent = onEvent
    }

    func start() {
        DispatchQueue.global(qos: .userInitiated).async { [self] in
            run()
        }
    }

    func requestStop() {
        lock.lock()
        isStopRequested = true
        lock.unlock()
    }

    func setVideoPreviewEnabled(_ isEnabled: Bool) {
        lock.lock()
        isVideoPreviewEnabled = isEnabled
        lock.unlock()
    }

    func waitUntilStopped(timeout: DispatchTime) -> Bool {
        stoppedSemaphore.wait(timeout: timeout) == .success
    }

    private func run() {
        defer {
            stoppedSemaphore.signal()
        }

        trackIRLogger.info("Opening TrackIR device for session \(self.sessionToken, privacy: .public)")

        var device: OpaquePointer?
        let openStatus = otir_tir5v3_open(&device)

        guard openStatus == OTIR_STATUS_OK, let device else {
            sendFailure(for: openStatus, operation: "Open")
            return
        }

        defer {
            trackIRLogger.info("Closing TrackIR device for session \(self.sessionToken, privacy: .public)")
            _ = otir_tir5v3_stop_streaming(device)
            otir_tir5v3_close(device)
        }

        var statuses = Array(
            repeating: otir_tir5v3_status(),
            count: Int(OTIR_TIR5V3_INIT_STATUS_COUNT)
        )
        var statusCount = 0

        let initializeStatus = statuses.withUnsafeMutableBufferPointer { buffer in
            otir_tir5v3_initialize(device, buffer.baseAddress, buffer.count, &statusCount)
        }
        guard initializeStatus == OTIR_STATUS_OK else {
            sendFailure(for: initializeStatus, operation: "Initialize")
            return
        }

        let startStatus = otir_tir5v3_start_streaming(device)
        guard startStatus == OTIR_STATUS_OK else {
            sendFailure(for: startStatus, operation: "Start streaming")
            return
        }

        send(.didStartStreaming)

        var frameIndex: UInt64 = 0
        var measuredFrameRate: Double?
        var sampledFrameCount = 0
        var sampleStartTime = CFAbsoluteTimeGetCurrent()
        var lastTelemetryPublishTime: CFAbsoluteTime?
        var lastPreviewPublishTime: CFAbsoluteTime?
        let frameWidth = Int(OTIR_TIR5V3_FRAME_WIDTH)
        let frameHeight = Int(OTIR_TIR5V3_FRAME_HEIGHT)
        let packetStorage = UnsafeMutablePointer<otir_tir5v3_packet>.allocate(capacity: 1)

        defer {
            packetStorage.deallocate()
        }

        while !stopRequested() {
            let readStatus = otir_tir5v3_read_packet(device, Int32(Self.readTimeoutMilliseconds), packetStorage)

            if readStatus == OTIR_STATUS_TIMEOUT {
                continue
            }

            guard readStatus == OTIR_STATUS_OK else {
                if !stopRequested() {
                    sendFailure(for: readStatus, operation: "Read packet")
                }
                return
            }

            guard packetTypeSupportsTrackIRFrame(packetStorage.pointee.packet_type) else {
                continue
            }

            frameIndex += 1
            sampledFrameCount += 1

            let now = CFAbsoluteTimeGetCurrent()
            let sampleDuration = now - sampleStartTime

            if sampleDuration >= 0.25 {
                measuredFrameRate = trackIRFrameRate(
                    frameCount: sampledFrameCount,
                    elapsedSeconds: sampleDuration
                )
                sampledFrameCount = 0
                sampleStartTime = now
            }

            var frameStats = otir_tir5v3_frame_stats()
            otir_tir5v3_packet_stats(packetStorage, frameIndex, &frameStats)

            let centroidX = frameStats.has_centroid ? frameStats.centroid_x : nil
            let centroidY = frameStats.has_centroid ? frameStats.centroid_y : nil
            let isVideoPreviewEnabled = videoPreviewEnabled()

            if isVideoPreviewEnabled {
                guard shouldPublishTrackIRTelemetry(
                    elapsedSinceLastPublish: lastPreviewPublishTime.map { now - $0 },
                    minimumInterval: trackIRPreviewFrameInterval(maximumFramesPerSecond: 30)
                ) else {
                    continue
                }

                var frameBytes = [UInt8](repeating: 0, count: frameWidth * frameHeight)
                frameBytes.withUnsafeMutableBufferPointer { buffer in
                    otir_tir5v3_build_frame(packetStorage, buffer.baseAddress, frameWidth)
                }

                guard let image = trackIRPreviewImage(
                    frameBytes: frameBytes,
                    width: frameWidth,
                    height: frameHeight
                ) else {
                    continue
                }

                send(.didRenderFrame(image, frameIndex, measuredFrameRate, centroidX, centroidY, packetStorage.pointee.packet_type))
                lastPreviewPublishTime = now
                continue
            }

            if shouldPublishTrackIRTelemetry(
                elapsedSinceLastPublish: lastTelemetryPublishTime.map { now - $0 },
                minimumInterval: trackIRTelemetryPublishInterval(isVideoEnabled: false)
            ) {
                send(.didUpdateTelemetry(frameIndex, measuredFrameRate, centroidX, centroidY, packetStorage.pointee.packet_type))
                lastTelemetryPublishTime = now
            }
        }

        trackIRLogger.info("TrackIR read loop stopped for session \(self.sessionToken, privacy: .public)")
    }

    private func stopRequested() -> Bool {
        lock.lock()
        let isStopRequested = isStopRequested
        lock.unlock()
        return isStopRequested
    }

    private func videoPreviewEnabled() -> Bool {
        lock.lock()
        let isVideoPreviewEnabled = isVideoPreviewEnabled
        lock.unlock()
        return isVideoPreviewEnabled
    }

    private func send(_ kind: TrackIRCameraEventKind) {
        guard !stopRequested() else {
            return
        }

        onEvent(TrackIRCameraEvent(sessionToken: sessionToken, kind: kind))
    }

    private func sendFailure(for status: otir_status, operation: String) {
        let message = trackIRFailureMessage(for: status, operation: operation)
        trackIRLogger.error("TrackIR \(operation, privacy: .public) failure for session \(self.sessionToken, privacy: .public): \(message, privacy: .public)")

        if status == OTIR_STATUS_NOT_FOUND {
            send(.didBecomeUnavailable(message))
            return
        }

        send(.didFail(message))
    }
}

func shouldStreamTrackIRSession(isTrackIREnabled: Bool, isVideoEnabled: Bool) -> Bool {
    isTrackIREnabled
}

func isRunningInXcodePreview(environment: [String: String]) -> Bool {
    environment["XCODE_RUNNING_FOR_PREVIEWS"] == "1" || environment["XCODE_RUNNING_FOR_PLAYGROUNDS"] == "1"
}

func shouldAccessTrackIRHardware(
    isTrackIREnabled: Bool,
    isVideoEnabled: Bool,
    environment: [String: String]
) -> Bool {
    shouldStreamTrackIRSession(
        isTrackIREnabled: isTrackIREnabled,
        isVideoEnabled: isVideoEnabled
    ) && !isRunningInXcodePreview(environment: environment)
}

func packetTypeSupportsTrackIRFrame(_ packetType: UInt8) -> Bool {
    packetType == 0x00 || packetType == 0x05
}

func trackIRFrameRate(frameCount: Int, elapsedSeconds: TimeInterval) -> Double? {
    guard frameCount > 0, elapsedSeconds > 0 else {
        return nil
    }

    return Double(frameCount) / elapsedSeconds
}

func trackIRTelemetryPublishInterval(isVideoEnabled: Bool) -> TimeInterval {
    isVideoEnabled ? 0 : 0.05
}

func trackIRPreviewFrameInterval(maximumFramesPerSecond: Double) -> TimeInterval {
    guard maximumFramesPerSecond > 0 else {
        return 0
    }

    return 1.0 / maximumFramesPerSecond
}

func shouldPublishTrackIRTelemetry(
    elapsedSinceLastPublish: TimeInterval?,
    minimumInterval: TimeInterval
) -> Bool {
    guard let elapsedSinceLastPublish else {
        return true
    }

    return elapsedSinceLastPublish >= minimumInterval
}

func trackIRFrameRateLabel(for frameRate: Double?) -> String {
    guard let frameRate else {
        return "-"
    }

    return "\(frameRate.formatted(.number.precision(.fractionLength(1)))) fps"
}

func trackIRCoordinateLabel(for coordinate: Double?) -> String {
    guard let coordinate else {
        return "-"
    }

    return String(Int(coordinate))
}

func trackIRCoordinatePairLabel(x: Double?, y: Double?) -> String {
    let xLabel = trackIRCoordinateLabel(for: x)
    let yLabel = trackIRCoordinateLabel(for: y)

    guard xLabel != "-", yLabel != "-" else {
        return "-"
    }

    return "\(xLabel), \(yLabel)"
}

func trackIRShutdownWaitMilliseconds(readTimeoutMilliseconds: Int) -> Int {
    max(250, readTimeoutMilliseconds * 4)
}

func trackIRPreviewImage(frameBytes: [UInt8], width: Int, height: Int) -> CGImage? {
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

func trackIRFailureMessage(for status: otir_status, operation: String) -> String {
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

func trackIRPreviewMessage(
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
