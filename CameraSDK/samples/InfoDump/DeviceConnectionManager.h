#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>

#include "cameramanager.h"  // CameraLibrary::cCameraManagerListener
#include "camera.h"         // CameraLibrary::Camera
#include "frame.h"          // CameraLibrary::Frame

/// Manages discovery and lifecycle for all devices exposed by the CameraLibrary
class DeviceConnectionManager : public CameraLibrary::cCameraManagerListener {
public:
    DeviceConnectionManager();
    ~DeviceConnectionManager();

    bool WaitForNewDevice(int timeout_ms);

    std::vector<std::shared_ptr<CameraLibrary::Camera>> GetDevices() const;

    std::shared_ptr<const CameraLibrary::Frame> GetLatestFrame(unsigned int serial) const;
    std::shared_ptr<const CameraLibrary::Frame> GetNextFrame(unsigned int serial) const;

    // Listener overrides
    void CameraInitialized() override;
    void CameraRemoved() override;

private:
    int  AddNewDeviceConnections();
    void RemoveAllDevices();
    void PruneDisconnectedDevices();
    static bool IsEsync(const CameraLibrary::Camera& dev);

private:
    std::vector<std::shared_ptr<CameraLibrary::Camera>> devices_;
    mutable std::mutex              notify_mutex_;
    std::condition_variable         notify_condition_;
    bool                            notify_{false};
};
