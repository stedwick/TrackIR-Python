#include "DeviceConnectionManager.h"
#include "cameralibrary.h"
#include "cameramanager.h"
#include "Core/UID.h"

#include <algorithm>
#include <chrono>

using namespace CameraLibrary;

DeviceConnectionManager::DeviceConnectionManager()
{
    CameraLibraryStartup();
    CameraManager::X().RegisterListener(this);
    CameraManager::X().ScanForCameras();
}

DeviceConnectionManager::~DeviceConnectionManager()
{
    RemoveAllDevices();
    CameraManager::X().UnregisterListener();
    CameraLibraryShutdown();
}

void DeviceConnectionManager::CameraInitialized()
{
    if (AddNewDeviceConnections() > 0) {
        std::lock_guard<std::mutex> lk(notify_mutex_);
        notify_ = true;
        notify_condition_.notify_one();
    }
}

void DeviceConnectionManager::CameraRemoved()
{
    PruneDisconnectedDevices();
}

int DeviceConnectionManager::AddNewDeviceConnections()
{
    int added = 0;
    {
        CameraList list;
        CameraManager::X().GetCameraList(list);

        for (int i = 0; i < list.Count(); ++i) {
            const auto& entry = list[i];
            const Core::cUID want_uid = entry.UID();

            const bool exists = std::any_of(
                devices_.begin(), devices_.end(),
                [&](const std::shared_ptr<Camera>& c){ return c && (c->UID() == want_uid); }
            );
            if (exists) continue;

            auto cam = CameraManager::X().GetCameraBySerial(entry.Serial());
            if (cam) {
                if (!cam->IsCameraRunning()) cam->Start();
                devices_.push_back(cam);
                ++added;
            }
        }
    }

    // ---- eSync via HardwareDeviceList ----
    {
        HardwareDeviceList device_list;
        int esync_index = -1;

        for (int i = 0; i < device_list.Count(); ++i) {
            std::string name(device_list[i].Name() ? device_list[i].Name() : "");
            if (name.find("eSync") != std::string::npos) {
                esync_index = i;
                break;
            }
        }

        if (esync_index >= 0) {
            const Core::cUID esync_uid = device_list[esync_index].UID();

            bool exists = std::any_of(
                devices_.begin(), devices_.end(),
                [&](const std::shared_ptr<Camera>& d){ return d && (d->UID() == esync_uid); }
            );

            if (!exists) {
                auto esync = CameraManager::X().GetDevice(esync_uid);
                if (esync && esync->IsSyncAuthority()) {
                    if (!esync->IsCameraRunning()) esync->Start();
                    devices_.push_back(esync);
                    ++added;
                }
            } else {
                for (auto it = devices_.begin(); it != devices_.end(); ++it) {
                    if ((*it)->UID() == esync_uid && !(*it)->IsSyncAuthority()) {
                        if ((*it)->IsCameraRunning()) (*it)->Stop();
                        devices_.erase(it);
                        break;
                    }
                }
            }
        } else {
            devices_.erase(
                std::remove_if(
                    devices_.begin(), devices_.end(),
                    [](const std::shared_ptr<Camera>& d){
                        return d && IsEsync(*d);
                    }
                ),
                devices_.end()
            );
        }
    }

    return added;
}

void DeviceConnectionManager::PruneDisconnectedDevices()
{
    using namespace CameraLibrary;

    std::vector<Core::cUID> present_uids;
    present_uids.reserve(devices_.size() + 2);
    {
        CameraList list;
        CameraManager::X().GetCameraList(list);
        for (int i = 0; i < list.Count(); ++i) {
            present_uids.push_back(list[i].UID());
        }
    }

    {
        HardwareDeviceList dev_list;
        for (int i = 0; i < dev_list.Count(); ++i) {
            const char* nm = dev_list[i].Name();
            const bool looks_esync = (nm && std::string(nm).find("eSync") != std::string::npos);
            if (!looks_esync) continue;

            const Core::cUID uid = dev_list[i].UID();
            auto es = CameraManager::X().GetDevice(uid);
            if (es && es->IsSyncAuthority()) {
                present_uids.push_back(uid);
            }
        }
    }

    auto contains_uid = [&](const Core::cUID& uid){
        return std::any_of(present_uids.begin(), present_uids.end(),
                           [&](const Core::cUID& x){ return x == uid; });
    };

    devices_.erase(
        std::remove_if(
            devices_.begin(), devices_.end(),
            [&](const std::shared_ptr<Camera>& d){
                if (!d) return true;
                const bool still_present = contains_uid(d->UID());
                if (!still_present) {
                    if (d->IsCameraRunning()) d->Stop();
                    return true;
                }
                return false;
            }
        ),
        devices_.end()
    );
}

void DeviceConnectionManager::RemoveAllDevices()
{
    for (auto& d : devices_) {
        if (d && d->IsCameraRunning()) d->Stop();
    }
    devices_.clear();
}

bool DeviceConnectionManager::WaitForNewDevice(int timeout_ms)
{
    std::unique_lock<std::mutex> lock(notify_mutex_);
    const bool ok = notify_condition_.wait_for(
        lock, std::chrono::milliseconds(timeout_ms), [this]{ return notify_; });
    if (ok) notify_ = false;
    return ok;
}

std::vector<std::shared_ptr<Camera>> DeviceConnectionManager::GetDevices() const
{
    return devices_;
}

std::shared_ptr<const Frame> DeviceConnectionManager::GetLatestFrame(unsigned int serial) const
{
    auto cam = CameraManager::X().GetCameraBySerial(serial);
    return cam ? cam->LatestFrame() : nullptr;
}

std::shared_ptr<const Frame> DeviceConnectionManager::GetNextFrame(unsigned int serial) const
{
    auto cam = CameraManager::X().GetCameraBySerial(serial);
    return cam ? cam->NextFrame() : nullptr;
}

bool DeviceConnectionManager::IsEsync(const Camera& dev)
{
    const char* n = dev.Name();
    if (!n) return false;
    return std::string(n).find("eSync") != std::string::npos;
}
