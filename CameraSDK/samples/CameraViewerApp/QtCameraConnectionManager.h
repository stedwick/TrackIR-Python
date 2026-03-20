#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <QObject>

#include "cameramanager.h"
#include "camera.h"
#include "frame.h"

class CameraConnectionManager
    : public QObject
    , public CameraLibrary::cCameraManagerListener
{
    Q_OBJECT
public:
    explicit CameraConnectionManager(QObject* parent = nullptr);
    ~CameraConnectionManager() override;

    bool WaitForNewCamera(int timeoutMs);

    std::vector<std::shared_ptr<CameraLibrary::Camera>> GetCameras() const;
    std::shared_ptr<CameraLibrary::Camera> GetCamera(unsigned int serial) const;
    std::shared_ptr<const CameraLibrary::Frame> GetLatestFrame(unsigned int serial) const;
    std::shared_ptr<const CameraLibrary::Frame> GetNextFrame(unsigned int serial) const;

    bool SetExposure(unsigned int serial, int value);
    bool SetFrameRate(unsigned int serial, int value);
    bool SetImagerGain(unsigned int serial, int value);
    bool SetColorGamma(unsigned int serial, float gamma);
    bool SetColorCompression(unsigned int serial, int mode, float quality, float bitrateScaled);
    bool SetVideoType(unsigned int serial, Core::eVideoMode mode, QString* outError = nullptr);

    void CameraInitialized() override;
    void CameraRemoved() override;

signals:
    void camerasChanged();
    
private:
    int  SyncCameraConnections();
    void RemoveAllCameras();

    std::vector<std::shared_ptr<CameraLibrary::Camera>> current_cameras;
    mutable std::mutex              notify_mutex;
    std::condition_variable         notify_condition;
    bool                            push_notification{false};
};
