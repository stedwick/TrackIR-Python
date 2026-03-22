import Foundation
import IOKit.hid

let xKeysFastMouseMultiplier = 2.5

enum XKeysIndicatorState: Equatable, Sendable {
    case disabled
    case notDetected
    case detectedIdle
    case pressed
}

struct XKeysMonitorSnapshot: Equatable, Sendable {
    var indicatorState: XKeysIndicatorState = .disabled
    var isPressed = false
}

nonisolated func isMatchingXKeysFootPedal(
    vendorID: Int,
    productID: Int,
    usagePage: Int,
    usage: Int
) -> Bool {
    vendorID == 0x05F3 &&
        [0x042C, 0x0438].contains(productID) &&
        usagePage == 0x000C &&
        usage == 0x0001
}

nonisolated func xKeysMiddlePedalPressed(report: [UInt8]) -> Bool {
    report.count > 2 && (report[2] & 0x04) != 0
}

nonisolated func xKeysIndicatorState(
    isEnabled: Bool,
    didDetectPedal: Bool,
    isPressed: Bool
) -> XKeysIndicatorState {
    guard isEnabled else {
        return .disabled
    }

    guard didDetectPedal else {
        return .notDetected
    }

    return isPressed ? .pressed : .detectedIdle
}

nonisolated func trackIRMouseEffectiveSpeed(
    baseSpeed: Double,
    isXKeysFastMouseEnabled: Bool,
    isXKeysPedalPressed: Bool,
    multiplier: Double = xKeysFastMouseMultiplier
) -> Double {
    guard isXKeysFastMouseEnabled, isXKeysPedalPressed else {
        return baseSpeed
    }

    return baseSpeed * multiplier
}

final class XKeysFootPedalMonitor: @unchecked Sendable {
    var onSnapshotChange: ((XKeysMonitorSnapshot) -> Void)?
    var onFailure: (() -> Void)?

    nonisolated var snapshot: XKeysMonitorSnapshot {
        lock.lock()
        defer { lock.unlock() }
        return snapshotState
    }

    private let lock = NSLock()
    private var manager: IOHIDManager?
    private var deviceEntries: [ObjectIdentifier: XKeysFootPedalDeviceEntry] = [:]
    private nonisolated(unsafe) var snapshotState = XKeysMonitorSnapshot()

    func setEnabled(_ isEnabled: Bool) {
        if isEnabled {
            startIfNeeded()
        } else {
            stop()
        }
    }

    func stop() {
        let cleanup = resetMonitor(to: .init())
        cleanup.teardown()
        publish(snapshot: cleanup.snapshot)
    }

    private func startIfNeeded() {
        lock.lock()
        let isAlreadyStarted = manager != nil
        lock.unlock()
        guard !isAlreadyStarted else {
            return
        }

        let manager = IOHIDManagerCreate(
            kCFAllocatorDefault,
            IOOptionBits(kIOHIDOptionsTypeNone)
        )
        let callbackContext = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
        let matching = [
            [
                kIOHIDVendorIDKey as String: 0x05F3,
                kIOHIDProductIDKey as String: 0x042C,
            ],
            [
                kIOHIDVendorIDKey as String: 0x05F3,
                kIOHIDProductIDKey as String: 0x0438,
            ],
        ] as CFArray

        IOHIDManagerSetDeviceMatchingMultiple(manager, matching)
        IOHIDManagerRegisterDeviceRemovalCallback(
            manager,
            xKeysDeviceRemovalCallback,
            callbackContext
        )
        IOHIDManagerScheduleWithRunLoop(
            manager,
            CFRunLoopGetMain(),
            CFRunLoopMode.defaultMode.rawValue
        )

        guard IOHIDManagerOpen(manager, IOOptionBits(kIOHIDOptionsTypeNone)) == kIOReturnSuccess else {
            fail(nextSnapshot: .init(indicatorState: .notDetected, isPressed: false))
            xKeysTeardown(manager: manager)
            return
        }

        let devices = xKeysDevices(manager: manager)
        var entries: [ObjectIdentifier: XKeysFootPedalDeviceEntry] = [:]

        for device in devices {
            guard let entry = XKeysFootPedalDeviceEntry(device: device) else {
                continue
            }

            guard attachDevice(entry, context: callbackContext) else {
                xKeysTeardown(entries: Array(entries.values))
                fail(nextSnapshot: .init(indicatorState: .notDetected, isPressed: false))
                xKeysTeardown(manager: manager)
                return
            }

            entries[ObjectIdentifier(device)] = entry
        }

        guard !entries.isEmpty else {
            fail(nextSnapshot: .init(indicatorState: .notDetected, isPressed: false))
            xKeysTeardown(manager: manager)
            return
        }

        let snapshot = XKeysMonitorSnapshot(
            indicatorState: .detectedIdle,
            isPressed: false
        )

        lock.lock()
        self.manager = manager
        deviceEntries = entries
        snapshotState = snapshot
        lock.unlock()

        publish(snapshot: snapshot)
    }

    private func attachDevice(
        _ entry: XKeysFootPedalDeviceEntry,
        context: UnsafeMutableRawPointer
    ) -> Bool {
        guard IOHIDDeviceOpen(entry.device, IOOptionBits(kIOHIDOptionsTypeNone)) == kIOReturnSuccess else {
            return false
        }

        IOHIDDeviceScheduleWithRunLoop(
            entry.device,
            CFRunLoopGetMain(),
            CFRunLoopMode.defaultMode.rawValue
        )
        IOHIDDeviceRegisterInputReportCallback(
            entry.device,
            entry.reportBuffer,
            entry.reportBufferLength,
            xKeysInputReportCallback,
            context
        )
        return true
    }

    fileprivate func handleInputReport(
        device: IOHIDDevice,
        result: IOReturn,
        report: UnsafeBufferPointer<UInt8>
    ) {
        guard result == kIOReturnSuccess else {
            fail(nextSnapshot: .init())
            return
        }

        let reportBytes = Array(report)
        let isPressed = xKeysMiddlePedalPressed(report: reportBytes)
        let deviceKey = ObjectIdentifier(device)
        var nextSnapshot: XKeysMonitorSnapshot?

        lock.lock()
        if let entry = deviceEntries[deviceKey] {
            entry.isPressed = isPressed
            let anyPressed = deviceEntries.values.contains(where: \.isPressed)
            let updatedSnapshot = XKeysMonitorSnapshot(
                indicatorState: xKeysIndicatorState(
                    isEnabled: true,
                    didDetectPedal: !deviceEntries.isEmpty,
                    isPressed: anyPressed
                ),
                isPressed: anyPressed
            )
            if updatedSnapshot != snapshotState {
                snapshotState = updatedSnapshot
                nextSnapshot = updatedSnapshot
            }
        }
        lock.unlock()

        if let nextSnapshot {
            publish(snapshot: nextSnapshot)
        }
    }

    fileprivate func handleDeviceRemoval(device: IOHIDDevice, result: IOReturn) {
        guard result == kIOReturnSuccess else {
            fail(nextSnapshot: .init())
            return
        }

        lock.lock()
        let isTrackedDevice = deviceEntries[ObjectIdentifier(device)] != nil
        lock.unlock()

        if isTrackedDevice {
            fail(nextSnapshot: .init())
        }
    }

    private func fail(nextSnapshot: XKeysMonitorSnapshot) {
        let cleanup = resetMonitor(to: nextSnapshot)
        cleanup.teardown()
        publish(snapshot: cleanup.snapshot)
        DispatchQueue.main.async { [onFailure] in
            onFailure?()
        }
    }

    private func resetMonitor(
        to nextSnapshot: XKeysMonitorSnapshot
    ) -> XKeysMonitorCleanup {
        lock.lock()
        defer { lock.unlock() }

        guard manager != nil || !deviceEntries.isEmpty || snapshotState != nextSnapshot else {
            return XKeysMonitorCleanup(
                manager: nil,
                entries: [],
                snapshot: snapshotState
            )
        }

        let cleanup = XKeysMonitorCleanup(
            manager: manager,
            entries: Array(deviceEntries.values),
            snapshot: nextSnapshot
        )
        manager = nil
        deviceEntries.removeAll()
        snapshotState = nextSnapshot
        return cleanup
    }

    private func publish(snapshot: XKeysMonitorSnapshot) {
        onSnapshotChange?(snapshot)
    }
}

private struct XKeysMonitorCleanup {
    let manager: IOHIDManager?
    let entries: [XKeysFootPedalDeviceEntry]
    let snapshot: XKeysMonitorSnapshot

    func teardown() {
        xKeysTeardown(entries: entries)
        xKeysTeardown(manager: manager)
    }
}

private final class XKeysFootPedalDeviceEntry {
    let device: IOHIDDevice
    let reportBuffer: UnsafeMutablePointer<UInt8>
    let reportBufferLength: CFIndex
    var isPressed = false

    init?(device: IOHIDDevice) {
        let maxReportSize = (IOHIDDeviceGetProperty(
            device,
            kIOHIDMaxInputReportSizeKey as CFString
        ) as? NSNumber)?.intValue ?? 64

        guard maxReportSize > 0 else {
            return nil
        }

        self.device = device
        reportBufferLength = CFIndex(maxReportSize)
        reportBuffer = .allocate(capacity: maxReportSize)
        reportBuffer.initialize(repeating: 0, count: maxReportSize)
    }

    deinit {
        reportBuffer.deinitialize(count: Int(reportBufferLength))
        reportBuffer.deallocate()
    }
}

private func xKeysDevices(manager: IOHIDManager) -> [IOHIDDevice] {
    guard let deviceSet = IOHIDManagerCopyDevices(manager) as? Set<IOHIDDevice> else {
        return []
    }

    return deviceSet.filter(xKeysDeviceMatches)
}

private func xKeysDeviceMatches(_ device: IOHIDDevice) -> Bool {
    let vendorID = (IOHIDDeviceGetProperty(device, kIOHIDVendorIDKey as CFString) as? NSNumber)?.intValue ?? 0
    let productID = (IOHIDDeviceGetProperty(device, kIOHIDProductIDKey as CFString) as? NSNumber)?.intValue ?? 0
    let usagePage = (IOHIDDeviceGetProperty(device, kIOHIDPrimaryUsagePageKey as CFString) as? NSNumber)?.intValue ?? 0
    let usage = (IOHIDDeviceGetProperty(device, kIOHIDPrimaryUsageKey as CFString) as? NSNumber)?.intValue ?? 0

    return isMatchingXKeysFootPedal(
        vendorID: vendorID,
        productID: productID,
        usagePage: usagePage,
        usage: usage
    )
}

private func xKeysTeardown(entries: [XKeysFootPedalDeviceEntry]) {
    for entry in entries {
        IOHIDDeviceUnscheduleFromRunLoop(
            entry.device,
            CFRunLoopGetMain(),
            CFRunLoopMode.defaultMode.rawValue
        )
        IOHIDDeviceClose(entry.device, IOOptionBits(kIOHIDOptionsTypeNone))
    }
}

private func xKeysTeardown(manager: IOHIDManager?) {
    guard let manager else {
        return
    }

    IOHIDManagerUnscheduleFromRunLoop(
        manager,
        CFRunLoopGetMain(),
        CFRunLoopMode.defaultMode.rawValue
    )
    IOHIDManagerClose(manager, IOOptionBits(kIOHIDOptionsTypeNone))
}

private func xKeysDeviceRemovalCallback(
    context: UnsafeMutableRawPointer?,
    result: IOReturn,
    sender: UnsafeMutableRawPointer?,
    device: IOHIDDevice?
) {
    guard
        let context,
        let device
    else {
        return
    }

    let monitor = Unmanaged<XKeysFootPedalMonitor>.fromOpaque(context).takeUnretainedValue()
    monitor.handleDeviceRemoval(device: device, result: result)
}

private func xKeysInputReportCallback(
    context: UnsafeMutableRawPointer?,
    result: IOReturn,
    sender: UnsafeMutableRawPointer?,
    type: IOHIDReportType,
    reportID: UInt32,
    report: UnsafeMutablePointer<UInt8>,
    reportLength: CFIndex
) {
    guard
        let context,
        reportLength > 0,
        let sender
    else {
        return
    }

    let monitor = Unmanaged<XKeysFootPedalMonitor>.fromOpaque(context).takeUnretainedValue()
    let device = Unmanaged<IOHIDDevice>.fromOpaque(sender).takeUnretainedValue()
    let reportBuffer = UnsafeBufferPointer(start: report, count: Int(reportLength))

    _ = type
    _ = reportID
    monitor.handleInputReport(device: device, result: result, report: reportBuffer)
}
