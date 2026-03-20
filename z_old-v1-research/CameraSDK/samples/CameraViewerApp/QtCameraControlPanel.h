#pragma once
#include <QWidget>
#include <QPointer>

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

class CameraConnectionManager;

class CameraControlPanel : public QWidget {
    Q_OBJECT
public:
    explicit CameraControlPanel(CameraConnectionManager* mgr, QWidget* parent = nullptr);
    void setSelectedSerial(unsigned serial) { selected_serial = serial; }

signals:
    void showWarning(const QString& title, const QString& message);

private:
    void buildUi();
    bool currentSerialValid() const;

    QPointer<CameraConnectionManager> camera_manager;
    unsigned selected_serial{0};

    QLineEdit* exposure_edit{nullptr};
    QPushButton* exposure_button{nullptr};

    QLineEdit* fps_edit{nullptr};
    QPushButton* fps_button{nullptr};

    QLineEdit* gain_edit{nullptr};
    QPushButton* gain_button{nullptr};

    QLineEdit* quality_edit{nullptr};
    QLineEdit* bitrate_edit{nullptr};
    QComboBox* mode_combo{nullptr};
    QPushButton* set_compression_button{nullptr};

    QLineEdit* gamma_edit{nullptr};
    QPushButton* gamma_button{nullptr};

    QWidget* mode_bar{nullptr};

private slots:
    void onSetExposure();
    void onSetFps();
    void onSetGain();
    void onSetGamma();
    void onSetCompression();
    void onSetVideoMode(int modeEnum);
};
