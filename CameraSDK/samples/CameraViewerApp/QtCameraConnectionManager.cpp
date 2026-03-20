#include <set>
#include <chrono>
#include <QMetaObject>

#include "QtCameraConnectionManager.h"
#include "cameralibrary.h"
#include "CameraHelpers.h"

// Specialized Qt Widget for sending camera commands from QtSignals

using namespace CameraLibrary;

CameraConnectionManager::CameraConnectionManager(QObject* parent)
    : QObject(parent)
{
    CameraLibraryStartup();
    CameraManager::X().RegisterListener(this);
    CameraManager::X().ScanForCameras();
    SyncCameraConnections();
}

CameraConnectionManager::~CameraConnectionManager()
{
    RemoveAllCameras();
    CameraManager::X().UnregisterListener();
    CameraLibraryShutdown();
}

int CameraConnectionManager::SyncCameraConnections()
{
    CameraList list;
    CameraManager::X().GetCameraList(list);

    std::set<unsigned int> present;
    for (int i = 0; i < list.Count(); ++i) {
        present.insert(list[i].Serial());
    }

    int changes = 0;

    // Remove cameras that disappeared
    for (auto it = current_cameras.begin(); it != current_cameras.end(); ) {
        unsigned s = (*it)->Serial();
        if (!present.count(s)) {
            if ((*it)->IsCameraRunning()) (*it)->Stop();
            it = current_cameras.erase(it);
            ++changes;
        } else {
            ++it;
        }
    }

    // Add cameras that appeared
    for (auto s : present) {
        bool exists = false;
        for (auto& c : current_cameras) if (c->Serial() == s) { exists = true; break; }
        if (!exists) {
            auto cam = CameraManager::X().GetCameraBySerial(s);
            if (cam) {
                cam->Start();
                current_cameras.push_back(cam);
                ++changes;
            }
        }
    }

    return changes;
}

void CameraConnectionManager::CameraInitialized()
{
    if (SyncCameraConnections() > 0) {
        QMetaObject::invokeMethod(this, [this]{ emit camerasChanged(); }, Qt::QueuedConnection);
        std::lock_guard<std::mutex> lock(notify_mutex);
        push_notification = true;
        notify_condition.notify_one();
    }
}

void CameraConnectionManager::CameraRemoved()
{
    if (SyncCameraConnections() > 0) {
        QMetaObject::invokeMethod(this, [this]{ emit camerasChanged(); }, Qt::QueuedConnection);
        std::lock_guard<std::mutex> lock(notify_mutex);
        push_notification = true;
        notify_condition.notify_one();
    }
}

bool CameraConnectionManager::WaitForNewCamera(int timeoutMs)
{
    std::unique_lock<std::mutex> lock(notify_mutex);
    bool ok = notify_condition.wait_for(
        lock, std::chrono::milliseconds(timeoutMs),
        [this]{ return push_notification; });
    if (ok) push_notification = false;
    return ok;
}

std::vector<std::shared_ptr<Camera>> CameraConnectionManager::GetCameras() const
{
    return current_cameras;
}

std::shared_ptr<const Frame> CameraConnectionManager::GetLatestFrame(unsigned int serial) const
{
    auto cam = CameraManager::X().GetCameraBySerial(serial);
    return cam ? cam->LatestFrame() : nullptr;
}

std::shared_ptr<const Frame> CameraConnectionManager::GetNextFrame(unsigned int serial) const
{
    auto cam = CameraManager::X().GetCameraBySerial(serial);
    return cam ? cam->NextFrame() : nullptr;
}

void CameraConnectionManager::RemoveAllCameras()
{
    for (auto& cam : current_cameras) {
        if (cam->IsCameraRunning()) cam->Stop();
    }
    current_cameras.clear();
}

std::shared_ptr<Camera> CameraConnectionManager::GetCamera(unsigned int serial) const {
    auto cam = CameraManager::X().GetCameraBySerial(serial);
    return cam;
}

bool CameraConnectionManager::SetExposure(unsigned int serial, int value) {
    auto cam = GetCamera(serial);
    if (!cam) return false;
    cam->SetExposure(value);
    return true;
}

bool CameraConnectionManager::SetFrameRate(unsigned int serial, int value) {
    auto cam = GetCamera(serial);
    if (!cam) return false;
    cam->SetFrameRate(value);
    return true;
}

bool CameraConnectionManager::SetImagerGain(unsigned int serial, int value) {
    auto cam = GetCamera(serial);
    if (!cam) return false;
    cam->SetImagerGain(static_cast<eImagerGain>(value));
    return true;
}

bool CameraConnectionManager::SetColorGamma(unsigned int serial, float gamma) {
    auto cam = GetCamera(serial);
    if (!cam) return false;
    if (!CameraHelper::IsPrimeColor(*cam)) return false; // not supported
    cam->SetColorGamma(gamma);
    return true;
}

bool CameraConnectionManager::SetColorCompression(unsigned int serial, int mode, float quality, float bitrateScaled) {
    auto cam = GetCamera(serial);
    if (!cam) return false;
    if (!CameraHelper::IsPrimeColor(*cam)) return false; // not supported
    cam->SetColorCompression(mode, quality, bitrateScaled);
    return true;
}

bool CameraConnectionManager::SetVideoType(unsigned int serial, Core::eVideoMode mode, QString* outError) {
    auto cam = GetCamera(serial);
    if (!cam) { if (outError) *outError = "No camera for that serial."; return false; }

    const bool isXCamera     = CameraHelper::IsXCamera(*cam);
    const bool isPrimeColor = CameraHelper::IsPrimeColor(*cam);

    // Duplex allowed only on X Cameras
    if (mode == Core::DuplexMode && !isXCamera) {
        if (outError) *outError = "Duplex mode is only supported on X cameras.";
        return false;
    }

    // Block non-color video mode switches for Prime Color
    if (isPrimeColor && mode != Core::VideoMode) {
        if (outError) *outError = "Prime Color cameras only support Color Video mode via compression controls.";
        return false;
    }

    cam->SetVideoType(mode);
    return true;
}
