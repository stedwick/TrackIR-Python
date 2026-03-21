//
//  ContentView.swift
//  OpenTrackIR
//
//  Created by Philip Brocoum on 3/21/26.
//

import KeyboardShortcuts
import SwiftUI

private let defaultControlValues = controlDefaultValues()

struct ContentView: View {
    @Environment(\.colorScheme) private var colorScheme
    @ObservedObject private var cameraController: TrackIRCameraController
    @AppStorage(ControlPreferenceKey.videoEnabled.rawValue) private var isVideoEnabled = defaultControlValues.videoEnabled
    @AppStorage(ControlPreferenceKey.trackIREnabled.rawValue) private var isTrackIREnabled = defaultControlValues.trackIREnabled
    @AppStorage(ControlPreferenceKey.mouseMovementEnabled.rawValue) private var isMouseMovementEnabled = defaultControlValues.mouseMovementEnabled
    @AppStorage(ControlPreferenceKey.mouseMovementSpeed.rawValue) private var mouseMovementSpeed = defaultControlValues.mouseMovementSpeed
    @AppStorage(ControlPreferenceKey.videoFlipHorizontal.rawValue) private var isVideoFlipHorizontalEnabled = defaultControlValues.videoFlipHorizontalEnabled
    @AppStorage(ControlPreferenceKey.videoFlipVertical.rawValue) private var isVideoFlipVerticalEnabled = defaultControlValues.videoFlipVerticalEnabled
    @AppStorage(ControlPreferenceKey.videoRotationDegrees.rawValue) private var videoRotationDegrees = defaultControlValues.videoRotationDegrees
    @AppStorage(ControlPreferenceKey.videoFramesPerSecond.rawValue) private var videoFramesPerSecond = defaultControlValues.videoFramesPerSecond

    @MainActor
    init() {
        _cameraController = ObservedObject(wrappedValue: TrackIRCameraController())
    }

    init(cameraController: TrackIRCameraController) {
        _cameraController = ObservedObject(wrappedValue: cameraController)
    }

    var body: some View {
        GeometryReader { geometry in
            let layout = dashboardLayout(for: geometry.size.width)
            let previewWidth = videoPreviewWidth(for: layout, width: geometry.size.width)
            let controlColumns = controlColumnCount(for: layout, width: geometry.size.width)

            ScrollView {
                VStack(alignment: .leading, spacing: 20) {
                    headerSection
                    videoPreview(previewWidth: previewWidth)
                        .frame(maxWidth: .infinity, alignment: .center)
                    controlSection(columnCount: controlColumns)
                }
                .frame(maxWidth: 1_120, alignment: .topLeading)
                .padding(24)
                .frame(maxWidth: .infinity, alignment: .top)
            }
            .background(appBackground)
            .safeAreaInset(edge: .bottom, spacing: 0) {
                scrollGlassHint
            }
        }
        .frame(minWidth: 760, minHeight: 560)
        .onAppear {
            syncTrackIRCamera()
        }
        .onChange(of: isTrackIREnabled) { _, _ in
            syncTrackIRCamera()
        }
        .onChange(of: isVideoEnabled) { _, _ in
            syncTrackIRCamera()
        }
        .onChange(of: videoFramesPerSecond) { _, _ in
            syncTrackIRCamera()
        }
        .onDisappear {
            cameraController.shutdown()
        }
    }

    private var isDarkMode: Bool {
        colorScheme == .dark
    }

    private var headerSection: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack(alignment: .top, spacing: 16) {
                VStack(alignment: .leading, spacing: 8) {
                    Text("Open · TrackIR")
                        .font(.system(size: 30, weight: .semibold, design: .rounded))

                    Text("macOS TrackIR preview and controls")
                        .font(.headline)
                        .foregroundStyle(.secondary)
                }

                Spacer(minLength: 16)

                Button(action: refreshTrackIRCamera) {
                    Label("Refresh", systemImage: "arrow.clockwise")
                }
                .buttonStyle(.borderedProminent)
                .controlSize(.regular)
            }

            HStack(spacing: 10) {
                statusChip(
                    title: isTrackIREnabled ? "TrackIR On" : "TrackIR Off",
                    systemImage: isTrackIREnabled ? "dot.radiowaves.left.and.right" : "power"
                )
                statusChip(
                    title: isVideoEnabled ? "Video Visible" : "Video Hidden",
                    systemImage: isVideoEnabled ? "video.fill" : "video.slash.fill"
                )
                statusChip(
                    title: isMouseMovementEnabled ? "Mouse On" : "Mouse Off",
                    systemImage: isMouseMovementEnabled ? "cursorarrow.motionlines" : "pause.circle.fill"
                )
            }
        }
    }

    private func videoPreview(previewWidth: CGFloat) -> some View {
        let videoTransform = previewVideoTransform(
            flipHorizontal: isVideoFlipHorizontalEnabled,
            flipVertical: isVideoFlipVerticalEnabled,
            rotationDegrees: videoRotationDegrees
        )
        let previewHeight = videoPreviewHeight(for: previewWidth)
        let telemetryWidth = previewTelemetryWidth(for: previewWidth)

        return VStack(spacing: 12) {
            VStack(spacing: 6) {
                Text(previewTitle)
                    .font(.title3.weight(.semibold))
                    .foregroundStyle(.primary)

                Text(previewMessage)
                    .font(.body)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)
            }

            ZStack {
                RoundedRectangle(cornerRadius: 24, style: .continuous)
                    .fill(
                        LinearGradient(
                            colors: [
                                Color(red: 0.10, green: 0.12, blue: 0.18),
                                Color(red: 0.04, green: 0.05, blue: 0.08),
                            ],
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )

                if isVideoEnabled, let previewImage = cameraController.previewImage {
                    Image(decorative: previewImage, scale: 1.0, orientation: .up)
                        .resizable()
                        .interpolation(.none)
                        .scaledToFill()
                        .scaleEffect(x: videoTransform.scaleX, y: videoTransform.scaleY)
                        .rotationEffect(.degrees(videoTransform.rotationDegrees))
                        .clipShape(RoundedRectangle(cornerRadius: 24, style: .continuous))
                }

                RoundedRectangle(cornerRadius: 24, style: .continuous)
                    .strokeBorder(previewBorderColor, lineWidth: 1)

                if cameraController.previewImage == nil {
                    Image(systemName: isVideoEnabled ? "video.fill" : "video.slash.fill")
                        .font(.system(size: 54, weight: .light))
                        .foregroundStyle(isVideoEnabled ? Color.green.opacity(0.9) : Color.secondary)
                }
            }
            .frame(width: previewWidth, height: previewHeight)
            .shadow(color: Color.black.opacity(0.22), radius: 18, x: 0, y: 10)

            HStack(spacing: 12) {
                previewMetric(title: "Source", value: cameraController.sourceLabel)
                previewMetric(title: "Rate", value: cameraController.frameRateLabel)
                previewMetric(title: "Position", value: cameraController.centroidPairLabel)
                previewMetric(title: "Backend", value: cameraController.backendLabel)
            }
            .frame(width: telemetryWidth)
        }
    }

    private func controlSection(columnCount: Int) -> some View {
        VStack(alignment: .leading, spacing: 18) {
            Text("Controls")
                .font(.title2.weight(.semibold))

            Text("Live controls for the TrackIR preview and desktop shell.")
                .font(.body)
                .foregroundStyle(.secondary)

            LazyVGrid(columns: gridColumns(count: columnCount), alignment: .leading, spacing: 14) {
                controlRow(
                    title: "Enable TrackIR",
                    detail: "Toggle TrackIR.",
                    systemImage: "dot.radiowaves.left.and.right",
                    isOn: $isTrackIREnabled
                )

                controlRow(
                    title: "Show Video",
                    detail: "Show the camera preview.",
                    systemImage: "video",
                    isOn: $isVideoEnabled
                )

                controlRow(
                    title: "Enable Mouse Movement",
                    detail: "Toggle mouse movement.",
                    systemImage: "cursorarrow.motionlines",
                    isOn: $isMouseMovementEnabled
                )

                mouseSpeedControlRow

                mouseHotkeyControlRow
                    .gridCellColumns(columnCount == 1 ? 1 : 2)

                videoControlsRow
                    .gridCellColumns(columnCount == 1 ? 1 : 2)
            }

            Divider()
                .padding(.vertical, 6)

            VStack(alignment: .leading, spacing: 8) {
                Text("Next Steps")
                    .font(.headline)
                Text("Use this live preview to validate the current libusb path before swapping the Mac transport later.")
                    .font(.body)
                    .foregroundStyle(.secondary)
            }
        }
        .padding(22)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(
            RoundedRectangle(cornerRadius: 22, style: .continuous)
                .fill(.regularMaterial)
        )
        .overlay(
            RoundedRectangle(cornerRadius: 22, style: .continuous)
                .strokeBorder(cardBorderColor, lineWidth: 1)
        )
    }

    private var scrollGlassHint: some View {
        Rectangle()
            .fill(.ultraThinMaterial)
            .mask {
                LinearGradient(
                    colors: [
                        Color.clear,
                        Color.black.opacity(0.55),
                        Color.black,
                    ],
                    startPoint: .top,
                    endPoint: .bottom
                )
            }
            .frame(height: 68)
            .allowsHitTesting(false)
    }

    private var previewMessage: String {
        trackIRPreviewMessage(
            isTrackIREnabled: isTrackIREnabled,
            isVideoEnabled: isVideoEnabled,
            phase: cameraController.phase,
            errorDescription: cameraController.lastErrorDescription
        )
    }

    private var previewTitle: String {
        if !isVideoEnabled {
            return "Video Preview Hidden"
        }

        switch cameraController.phase {
            case .streaming:
                return "Live TrackIR Camera"
            case .starting:
                return "Starting TrackIR Camera"
            case .unavailable:
                return "TrackIR Not Found"
            case .failed:
                return "TrackIR Camera Error"
            case .idle:
                return isTrackIREnabled ? "Video Preview Ready" : "TrackIR Off"
        }
    }

    private var appBackground: some View {
        LinearGradient(
            colors: [
                isDarkMode
                    ? Color(red: 0.08, green: 0.10, blue: 0.14)
                    : Color(red: 0.93, green: 0.95, blue: 0.98),
                isDarkMode
                    ? Color(red: 0.12, green: 0.15, blue: 0.22)
                    : Color(red: 0.86, green: 0.90, blue: 0.95),
            ],
            startPoint: .topLeading,
            endPoint: .bottomTrailing
        )
        .ignoresSafeArea()
    }

    private var chipFillColor: Color {
        isDarkMode ? Color.white.opacity(0.08) : Color.white.opacity(0.72)
    }

    private var chipBorderColor: Color {
        isDarkMode ? Color.white.opacity(0.12) : Color.black.opacity(0.06)
    }

    private var cardBorderColor: Color {
        isDarkMode ? Color.white.opacity(0.12) : Color.black.opacity(0.08)
    }

    private var previewBorderColor: Color {
        isDarkMode ? Color.white.opacity(0.10) : Color.white.opacity(0.16)
    }

    private var rowFillColor: Color {
        isDarkMode ? Color.white.opacity(0.06) : Color.white.opacity(0.64)
    }

    private func gridColumns(count: Int) -> [GridItem] {
        Array(repeating: GridItem(.flexible(minimum: 180), spacing: 14, alignment: .top), count: count)
    }

    private func statusChip(title: String, systemImage: String) -> some View {
        HStack(spacing: 8) {
            Image(systemName: systemImage)
                .font(.subheadline.weight(.medium))
                .frame(width: 16, height: 16)

            Text(title)
                .font(.subheadline.weight(.medium))
        }
            .padding(.horizontal, 12)
            .padding(.vertical, 7)
            .frame(height: 34)
            .background(
                Capsule(style: .continuous)
                    .fill(chipFillColor)
            )
            .overlay(
                Capsule(style: .continuous)
                    .strokeBorder(chipBorderColor, lineWidth: 1)
            )
    }

    private func previewMetric(title: String, value: String) -> some View {
        VStack(spacing: 4) {
            Text(title.uppercased())
                .font(.caption.weight(.semibold))
                .foregroundStyle(.white.opacity(0.55))
            Text(value)
                .font(.callout.weight(.medium))
                .foregroundStyle(.white)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 12)
        .padding(.horizontal, 10)
        .background(Color.white.opacity(0.08))
        .clipShape(RoundedRectangle(cornerRadius: 14, style: .continuous))
    }

    private func controlRow(
        title: String,
        detail: String,
        systemImage: String,
        isOn: Binding<Bool>
    ) -> some View {
        controlCard {
            HStack(alignment: .top, spacing: 14) {
                controlCopy(title: title, detail: detail, systemImage: systemImage)

                Spacer(minLength: 16)

                Toggle("", isOn: isOn)
                    .labelsHidden()
                    .toggleStyle(.switch)
            }
        }
    }

    private var mouseSpeedControlRow: some View {
        controlCard {
            VStack(alignment: .leading, spacing: 14) {
                HStack(alignment: .top, spacing: 14) {
                    controlCopy(
                        title: "Mouse Speed",
                        detail: "Adjust cursor speed.",
                        systemImage: "speedometer"
                    )

                    Spacer(minLength: 16)

                    Text(mouseSpeedValueLabel(for: mouseMovementSpeed))
                        .font(.title3.weight(.semibold))
                        .monospacedDigit()
                }

                VStack(alignment: .leading, spacing: 8) {
                    Slider(value: $mouseMovementSpeed, in: 0.25 ... 3.0, step: 0.25)

                    HStack {
                        Text("0.25x")
                        Spacer()
                        Text("3x")
                    }
                    .font(.caption)
                    .foregroundStyle(.secondary)
                }
            }
        }
    }

    private var mouseHotkeyControlRow: some View {
        controlCard {
            VStack(alignment: .leading, spacing: 14) {
                controlCopy(
                    title: "Mouse Toggle Hotkey",
                    detail: "Choose the toggle shortcut.",
                    systemImage: "keyboard"
                )

                KeyboardShortcuts.Recorder(for: .toggleMouseMovement)
                    .frame(maxWidth: .infinity, alignment: .center)

                Text("Shortcut hookup comes later.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
    }

    private var videoControlsRow: some View {
        controlCard {
            VStack(alignment: .leading, spacing: 16) {
                controlCopy(
                    title: "Video Controls",
                    detail: "Adjust the preview.",
                    systemImage: "camera.filters"
                )

                VStack(alignment: .leading, spacing: 12) {
                    Toggle("Flip Horizontal", isOn: $isVideoFlipHorizontalEnabled)
                        .toggleStyle(.checkbox)

                    Toggle("Flip Vertical", isOn: $isVideoFlipVerticalEnabled)
                        .toggleStyle(.checkbox)
                }

                VStack(alignment: .leading, spacing: 14) {
                    HStack(alignment: .top, spacing: 14) {
                        controlCopy(
                            title: "Rotate",
                            detail: "Rotate the preview.",
                            systemImage: "rotate.right"
                        )

                        Spacer(minLength: 16)

                        Text(videoRotationValueLabel(for: videoRotationDegrees))
                            .font(.title3.weight(.semibold))
                            .monospacedDigit()
                    }

                    VStack(alignment: .leading, spacing: 8) {
                        Slider(value: $videoRotationDegrees, in: 0 ... 360, step: 1)

                        HStack {
                            Text("0°")
                            Spacer()
                            Text("360°")
                        }
                        .font(.caption)
                        .foregroundStyle(.secondary)
                    }
                }

                VStack(alignment: .leading, spacing: 14) {
                    HStack(alignment: .top, spacing: 14) {
                        controlCopy(
                            title: "TrackIR FPS",
                            detail: "Cap TrackIR telemetry.",
                            systemImage: "gauge.with.dots.needle.33percent"
                        )

                        Spacer(minLength: 16)

                        Text(trackIRFramesPerSecondValueLabel(for: videoFramesPerSecond))
                            .font(.title3.weight(.semibold))
                            .monospacedDigit()
                    }

                    VStack(alignment: .leading, spacing: 8) {
                        Slider(value: $videoFramesPerSecond, in: 0 ... 125, step: 1)

                        HStack {
                            Text("0")
                            Spacer()
                            Text("125")
                        }
                        .font(.caption)
                        .foregroundStyle(.secondary)
                    }
                }
            }
        }
    }

    private func controlCopy(title: String, detail: String, systemImage: String) -> some View {
        HStack(alignment: .top, spacing: 14) {
            Image(systemName: systemImage)
                .font(.title3)
                .foregroundStyle(Color.accentColor)
                .frame(width: 28, height: 28)
                .padding(8)
                .background(Color.accentColor.opacity(0.12))
                .clipShape(RoundedRectangle(cornerRadius: 12, style: .continuous))

            VStack(alignment: .leading, spacing: 4) {
                Text(title)
                    .font(.headline)
                Text(detail)
                    .font(.subheadline)
                    .foregroundStyle(.secondary)
            }
        }
    }

    private func controlCard<Content: View>(@ViewBuilder content: () -> Content) -> some View {
        content()
            .padding(18)
            .frame(maxWidth: .infinity, alignment: .topLeading)
            .background(rowFillColor)
            .clipShape(RoundedRectangle(cornerRadius: 18, style: .continuous))
            .overlay(
                RoundedRectangle(cornerRadius: 18, style: .continuous)
                    .strokeBorder(cardBorderColor, lineWidth: 1)
            )
    }

    private func syncTrackIRCamera() {
        cameraController.syncStreaming(
            isTrackIREnabled: isTrackIREnabled,
            isVideoEnabled: isVideoEnabled,
            maximumTrackingFramesPerSecond: videoFramesPerSecond
        )
    }

    private func refreshTrackIRCamera() {
        cameraController.refresh(
            isTrackIREnabled: isTrackIREnabled,
            isVideoEnabled: isVideoEnabled,
            maximumTrackingFramesPerSecond: videoFramesPerSecond
        )
    }
}

enum DashboardLayoutMode: Equatable {
    case stacked
    case twoColumn
}

enum ControlPreferenceKey: String {
    case videoEnabled = "contentView.videoEnabled"
    case trackIREnabled = "contentView.trackIREnabled"
    case mouseMovementEnabled = "contentView.mouseMovementEnabled"
    case mouseMovementSpeed = "contentView.mouseMovementSpeed"
    case videoFlipHorizontal = "contentView.videoFlipHorizontal"
    case videoFlipVertical = "contentView.videoFlipVertical"
    case videoRotationDegrees = "contentView.videoRotationDegrees"
    case videoFramesPerSecond = "contentView.videoFramesPerSecond"
}

struct ControlDefaultValues: Equatable {
    let videoEnabled: Bool
    let trackIREnabled: Bool
    let mouseMovementEnabled: Bool
    let mouseMovementSpeed: Double
    let videoFlipHorizontalEnabled: Bool
    let videoFlipVerticalEnabled: Bool
    let videoRotationDegrees: Double
    let videoFramesPerSecond: Double
}

struct VideoPreviewTransform: Equatable {
    let scaleX: CGFloat
    let scaleY: CGFloat
    let rotationDegrees: Double
}

func defaultMouseMovementShortcut() -> KeyboardShortcuts.Shortcut {
    .init(.f7, modifiers: [.shift])
}

func controlDefaultValues() -> ControlDefaultValues {
    ControlDefaultValues(
        videoEnabled: false,
        trackIREnabled: false,
        mouseMovementEnabled: false,
        mouseMovementSpeed: 1.0,
        videoFlipHorizontalEnabled: true,
        videoFlipVerticalEnabled: false,
        videoRotationDegrees: 0.0,
        videoFramesPerSecond: 60.0
    )
}

func toggledMouseMovementState(isEnabled: Bool) -> Bool {
    !isEnabled
}

func mouseSpeedValueLabel(for speed: Double) -> String {
    "\(speed.formatted(.number.precision(.fractionLength(0 ... 2))))x"
}

func previewVideoTransform(
    flipHorizontal: Bool,
    flipVertical: Bool,
    rotationDegrees: Double
) -> VideoPreviewTransform {
    VideoPreviewTransform(
        scaleX: flipHorizontal ? -1 : 1,
        scaleY: flipVertical ? -1 : 1,
        rotationDegrees: normalizedRotationDegrees(rotationDegrees)
    )
}

func normalizedRotationDegrees(_ rotationDegrees: Double) -> Double {
    let normalized = rotationDegrees.truncatingRemainder(dividingBy: 360)
    return normalized >= 0 ? normalized : normalized + 360
}

func videoRotationValueLabel(for rotationDegrees: Double) -> String {
    "\(Int(normalizedRotationDegrees(rotationDegrees).rounded()))°"
}

func trackIRFramesPerSecondValueLabel(for framesPerSecond: Double) -> String {
    guard framesPerSecond > 0 else {
        return "Uncapped"
    }

    return "\(Int(framesPerSecond.rounded())) fps"
}

extension KeyboardShortcuts.Name {
    static let toggleMouseMovement = Self("toggleMouseMovement", default: defaultMouseMovementShortcut())
}

func dashboardLayout(for width: CGFloat) -> DashboardLayoutMode {
    width >= 960 ? .twoColumn : .stacked
}

func videoPreviewWidth(for layout: DashboardLayoutMode, width: CGFloat) -> CGFloat {
    switch layout {
        case .twoColumn:
            return min(320, max(260, width * 0.24))
        case .stacked:
            return min(340, max(240, width * 0.38))
    }
}

func videoPreviewHeight(for width: CGFloat) -> CGFloat {
    width * 0.75
}

func previewTelemetryWidth(for previewWidth: CGFloat) -> CGFloat {
    max(previewWidth, 620)
}

func controlColumnCount(for layout: DashboardLayoutMode, width: CGFloat) -> Int {
    switch layout {
        case .twoColumn:
            return width >= 900 ? 2 : 1
        case .stacked:
            return width >= 720 ? 2 : 1
    }
}

#Preview {
    ContentView()
}
