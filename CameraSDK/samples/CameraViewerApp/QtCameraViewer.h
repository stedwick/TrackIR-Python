#pragma once

#include <QWidget>
#include <memory>
#include <optional>
#include <atomic>
#include <mutex>

#ifdef HAVE_FFMPEG
#include "videodecoder.h"
#endif

// Forward Declarations
class QLabel;
class QStackedLayout;
class CameraPicker;
class CameraControlPanel;
class VideoWidget;
class CameraConnectionManager;
namespace CameraLibrary { class Camera; }
namespace CameraHelper { class FrameRateCalculator; }

class QtCameraViewer : public QWidget
{
    Q_OBJECT
public:
    QtCameraViewer(CameraConnectionManager* mgr,
                   std::mutex& camMutex,
                   std::shared_ptr<CameraLibrary::Camera>& currentCamera,
                   std::atomic<uint64_t>& switchEpoch,
                   std::atomic<unsigned>&  activeSerial,
                   CameraHelper::FrameRateCalculator& fpsCalc,
                   QWidget* parent = nullptr);

    static void ApplyAppStyle();

    QWidget*     videoContainer() const { return viewer_container; }
    VideoWidget* videoWidget()    const { return gl_viewer_window; }

private:
    CameraPicker*   camera_picker{nullptr};
    CameraControlPanel* camera_controls{nullptr};
    QWidget*          status_bar{nullptr};
    QLabel*           fps_label{nullptr};
    QWidget*          center_widget{nullptr};
    QStackedLayout*   stacked_layout{nullptr};
    QWidget*          empty_pane{nullptr};
    QWidget*          viewer_container{nullptr};
    VideoWidget*      gl_viewer_window{nullptr};

    CameraConnectionManager* camera_manager{nullptr};
    std::mutex&              camera_mutex;
    std::shared_ptr<CameraLibrary::Camera>& current_camera;
    std::atomic<uint64_t>&   switch_epoch;
    std::atomic<unsigned>&   active_serial;
    CameraHelper::FrameRateCalculator& fps_calculator;

    void buildUi();
    void wireSignals();
    void setEmptyState(bool anyCamerasPresent);
    void handleSerialSelected(std::optional<unsigned> serialOpt);
};
