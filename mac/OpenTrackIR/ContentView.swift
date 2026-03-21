//
//  ContentView.swift
//  OpenTrackIR
//
//  Created by Philip Brocoum on 3/21/26.
//

import SwiftUI

struct ContentView: View {
    @Environment(\.colorScheme) private var colorScheme
    @State private var isVideoEnabled = true
    @State private var isTrackIREnabled = true
    @State private var isMouseMovementEnabled = true

    var body: some View {
        GeometryReader { geometry in
            let layout = dashboardLayout(for: geometry.size.width)
            let previewWidth = videoPreviewWidth(for: layout, width: geometry.size.width)
            let controlColumns = controlColumnCount(for: layout, width: geometry.size.width)

            ScrollView {
                VStack(alignment: .leading, spacing: 20) {
                    headerSection
                    videoPreview
                        .frame(width: previewWidth)
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
    }

    private var isDarkMode: Bool {
        colorScheme == .dark
    }

    private var headerSection: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("OpenTrackIR")
                .font(.system(size: 30, weight: .semibold, design: .rounded))

            Text("macOS preview shell for TrackIR video and controls")
                .font(.headline)
                .foregroundStyle(.secondary)

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

    private var videoPreview: some View {
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

            RoundedRectangle(cornerRadius: 24, style: .continuous)
                .strokeBorder(previewBorderColor, lineWidth: 1)

            VStack(spacing: 18) {
                Image(systemName: isVideoEnabled ? "video.fill" : "video.slash.fill")
                    .font(.system(size: 54, weight: .light))
                    .foregroundStyle(isVideoEnabled ? Color.green.opacity(0.9) : Color.secondary)

                VStack(spacing: 6) {
                    Text(isVideoEnabled ? "Video Preview Ready" : "Video Preview Hidden")
                        .font(.title3.weight(.semibold))
                        .foregroundStyle(.white)

                    Text(previewMessage)
                        .font(.body)
                        .foregroundStyle(.white.opacity(0.68))
                        .multilineTextAlignment(.center)
                }

                HStack(spacing: 12) {
                    previewMetric(title: "Source", value: "Native macOS view")
                    previewMetric(title: "Target Feed", value: "640×480")
                    previewMetric(title: "Backend", value: "Not connected yet")
                }
            }
            .padding(28)
        }
        .aspectRatio(4.0 / 3.0, contentMode: .fit)
        .shadow(color: Color.black.opacity(0.22), radius: 18, x: 0, y: 10)
    }

    private func controlSection(columnCount: Int) -> some View {
        VStack(alignment: .leading, spacing: 18) {
            Text("Controls")
                .font(.title2.weight(.semibold))

            Text("UI-only controls for the first macOS shell. These toggles do not talk to the device yet.")
                .font(.body)
                .foregroundStyle(.secondary)

            LazyVGrid(columns: gridColumns(count: columnCount), alignment: .leading, spacing: 14) {
                controlRow(
                    title: "Enable TrackIR",
                    detail: "Turn the future device session on or off from the macOS app.",
                    systemImage: "dot.radiowaves.left.and.right",
                    isOn: $isTrackIREnabled
                )

                controlRow(
                    title: "Show Video",
                    detail: "Display the future camera preview inside the native macOS panel.",
                    systemImage: "video",
                    isOn: $isVideoEnabled
                )

                controlRow(
                    title: "Enable Mouse Movement",
                    detail: "Allow future mouse output to follow head tracking when the backend is connected.",
                    systemImage: "cursorarrow.motionlines",
                    isOn: $isMouseMovementEnabled
                )
                .gridCellColumns(columnCount == 1 ? 1 : 2)
            }

            Divider()
                .padding(.vertical, 6)

            VStack(alignment: .leading, spacing: 8) {
                Text("Next Steps")
                    .font(.headline)
                Text("Wire this shell to the shared C library, then replace the placeholder preview with a native frame source.")
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
        if !isVideoEnabled {
            return "Turn video back on to show the future camera feed in this native preview surface."
        }

        if isTrackIREnabled {
            return "The layout is ready for a live native frame source once the device bridge is connected."
        }

        return "TrackIR is still off. This panel is the placeholder for the eventual live camera preview."
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
        Label(title, systemImage: systemImage)
            .font(.subheadline.weight(.medium))
            .padding(.horizontal, 12)
            .padding(.vertical, 7)
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

            Spacer(minLength: 16)

            Toggle("", isOn: isOn)
                .labelsHidden()
                .toggleStyle(.switch)
        }
        .padding(18)
        .frame(maxWidth: .infinity, alignment: .topLeading)
        .background(rowFillColor)
        .clipShape(RoundedRectangle(cornerRadius: 18, style: .continuous))
        .overlay(
            RoundedRectangle(cornerRadius: 18, style: .continuous)
                .strokeBorder(cardBorderColor, lineWidth: 1)
        )
    }
}

enum DashboardLayoutMode: Equatable {
    case stacked
    case twoColumn
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
