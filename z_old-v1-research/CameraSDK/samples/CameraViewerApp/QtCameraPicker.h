#pragma once

#include <QWidget>
#include <optional>

class QComboBox;
class QLabel;
class CameraConnectionManager;

class CameraPicker : public QWidget
{
    Q_OBJECT
public:
    explicit CameraPicker(CameraConnectionManager* mgr, QWidget* parent=nullptr);

    std::optional<unsigned> selectedSerial() const { return selected_serial; }
    void setSelectedSerial(std::optional<unsigned> s);

    QComboBox* combo() const { return combo_box; }

signals:
    void serialChanged(std::optional<unsigned> serial);
    void camerasPresentChanged(bool any);

private slots:
    void onComboIndexChanged(int idx);
    void onCamerasChanged();

private:
    void refresh();

    CameraConnectionManager* camera_manager{nullptr};
    QLabel*     picker_label{nullptr};
    QComboBox*  combo_box{nullptr};
    std::optional<unsigned> selected_serial;
};

