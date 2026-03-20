#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QIntValidator>
#include <QDoubleValidator>

#include "QtCameraControlPanel.h"
#include "QtCameraConnectionManager.h"
#include "CameraHelpers.h"
#include "cameralibrary.h"

using namespace CameraLibrary;

// Specialized collection of widgets for camera controls

CameraControlPanel::CameraControlPanel(CameraConnectionManager* mgr, QWidget* parent)
    : QWidget(parent), camera_manager(mgr) {
    buildUi();
    connect(this, &CameraControlPanel::showWarning, this, [](const QString& t, const QString& m){
        QMessageBox::warning(nullptr, t, m);
    });
}

bool CameraControlPanel::currentSerialValid() const {
    return selected_serial != 0;
}

void CameraControlPanel::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(6);

    // Row: Exposure / FPS / Gain
    auto* row1 = new QWidget(this);
    auto* h1   = new QHBoxLayout(row1); h1->setContentsMargins(0,0,0,0);

    auto* camLbl = new QLabel("Controls:", row1);
    exposure_edit = new QLineEdit(row1);
    exposure_edit->setPlaceholderText("Exposure");
    exposure_edit->setValidator(new QIntValidator(1, 10000, exposure_edit));
    exposure_button = new QPushButton("Set Exposure", row1);
    connect(exposure_button, &QPushButton::clicked, this, &CameraControlPanel::onSetExposure);

    fps_edit = new QLineEdit(row1);
    fps_edit->setPlaceholderText("Frame Rate");
    fps_edit->setValidator(new QIntValidator(1, 1000, fps_edit));
    fps_button  = new QPushButton("Set Frame Rate", row1);
    connect(fps_button, &QPushButton::clicked, this, &CameraControlPanel::onSetFps);

    gain_edit = new QLineEdit(row1);
    gain_edit->setPlaceholderText("Gain");
    gain_edit->setValidator(new QIntValidator(0, 7, gain_edit));
    gain_button  = new QPushButton("Set Gain", row1);
    connect(gain_button, &QPushButton::clicked, this, &CameraControlPanel::onSetGain);

    h1->addWidget(camLbl);
    h1->addWidget(exposure_edit);
    h1->addWidget(exposure_button);
    h1->addSpacing(8);
    h1->addWidget(fps_edit);
    h1->addWidget(fps_button);
    h1->addWidget(gain_edit);
    h1->addWidget(gain_button);
    root->addWidget(row1);

    // Row: Video modes
    mode_bar = new QWidget(this);
    auto* hm  = new QHBoxLayout(mode_bar); hm->setContentsMargins(0,0,0,0); hm->setSpacing(6);

    auto addModeBtn = [&](const char* text, Core::eVideoMode mode) {
        auto* b = new QPushButton(text, mode_bar);
        connect(b, &QPushButton::clicked, this, [this, mode]{ onSetVideoMode(int(mode)); });
        hm->addWidget(b);
    };
    addModeBtn("Segment",   Core::SegmentMode);
    addModeBtn("Grayscale", Core::GrayscaleMode);
    addModeBtn("Object",    Core::ObjectMode);
    addModeBtn("Precision", Core::PrecisionMode);
    addModeBtn("MJPEG",     Core::MJPEGMode);
    addModeBtn("Duplex",    Core::DuplexMode);
    root->addWidget(mode_bar);

    // Row: Color compression / gamma
    auto* row2 = new QWidget(this);
    auto* h2   = new QHBoxLayout(row2); h2->setContentsMargins(0,0,0,0); h2->setSpacing(6);

    auto* compLbl = new QLabel("Color Compression:", row2);

    quality_edit = new QLineEdit(row2);
    quality_edit->setPlaceholderText("Quality (0.0–1.0)");
    auto* qv = new QDoubleValidator(0.0, 1.0, 3, quality_edit);
    qv->setNotation(QDoubleValidator::StandardNotation);
    quality_edit->setValidator(qv);

    bitrate_edit = new QLineEdit(row2);
    bitrate_edit->setPlaceholderText("Bitrate (Mbps)");
    auto* bv = new QDoubleValidator(0.0, 10000.0, 2, bitrate_edit);
    bv->setNotation(QDoubleValidator::StandardNotation);
    bitrate_edit->setValidator(bv);

    mode_combo = new QComboBox(row2);
    mode_combo->addItem("Variable Bitrate", QVariant(0));
    mode_combo->addItem("Constant Bitrate", QVariant(1));

    set_compression_button = new QPushButton("Set Color Compression", row2);
    connect(set_compression_button, &QPushButton::clicked, this, &CameraControlPanel::onSetCompression);

    gamma_edit = new QLineEdit(row2);
    gamma_edit->setPlaceholderText("Gamma (0.0–1.0)");
    gamma_edit->setValidator(new QDoubleValidator(0.1, 1.0, 3, gamma_edit));
    gamma_button = new QPushButton("Set Gamma", row2);
    connect(gamma_button, &QPushButton::clicked, this, &CameraControlPanel::onSetGamma);

    h2->addWidget(compLbl);
    h2->addWidget(quality_edit);
    h2->addWidget(bitrate_edit);
    h2->addWidget(mode_combo);
    h2->addWidget(set_compression_button);
    h2->addWidget(gamma_edit);
    h2->addWidget(gamma_button);
    root->addWidget(row2);
}

void CameraControlPanel::onSetExposure() {
    if (!currentSerialValid()) { emit showWarning("No Camera", "No camera is currently selected."); return; }
    bool ok=false; const int v = exposure_edit->text().toInt(&ok);
    if (!ok) { emit showWarning("Invalid Value","Enter a valid integer for Exposure."); return; }
    if (!camera_manager->SetExposure(selected_serial, v)) {
        emit showWarning("Failed", "Could not set exposure on the selected camera.");
    }
}

void CameraControlPanel::onSetFps() {
    if (!currentSerialValid()) { emit showWarning("No Camera", "No camera is currently selected."); return; }
    bool ok=false; const int v = fps_edit->text().toInt(&ok);
    if (!ok) { emit showWarning("Invalid Value","Enter a valid integer for Frame Rate."); return; }
    if (!camera_manager->SetFrameRate(selected_serial, v)) {
        emit showWarning("Failed", "Could not set frame rate on the selected camera.");
    }
}

void CameraControlPanel::onSetGain() {
    if (!currentSerialValid()) { emit showWarning("No Camera", "No camera is currently selected."); return; }
    bool ok=false; const int v = gain_edit->text().toInt(&ok);
    if (!ok) { emit showWarning("Invalid Value","Enter a valid integer for Gain."); return; }
    if (!camera_manager->SetImagerGain(selected_serial, v)) {
        emit showWarning("Failed", "Could not set imager gain on the selected camera.");
    }
}

void CameraControlPanel::onSetGamma() {
    if (!currentSerialValid()) { emit showWarning("No Camera", "No camera is currently selected."); return; }
    bool ok=false; const float g = gamma_edit->text().toFloat(&ok);
    if (!ok) { emit showWarning("Invalid Value","Enter a number 0.1–1.0 for Gamma."); return; }
    if (!camera_manager->SetColorGamma(selected_serial, g)) {
        emit showWarning("Unsupported Camera", "Color gamma is only supported on Prime Color cameras.");
    }
}

void CameraControlPanel::onSetCompression() {
    if (!currentSerialValid()) { emit showWarning("No Camera", "No camera is currently selected."); return; }
    bool okQ=false, okB=false;
    const float quality = quality_edit->text().toFloat(&okQ);
    const double mbps   = bitrate_edit->text().toDouble(&okB);
    if (!okQ || quality < 0.0f || quality > 1.0f) {
        emit showWarning("Invalid Quality", "Quality must be 0.0–1.0."); return;
    }
    if (!okB || mbps < 0.0) {
        emit showWarning("Invalid Bitrate", "Bitrate (Mbps) must be ≥ 0."); return;
    }
    const float bitrateScaled = CameraHelper::MbpsToNormalized(float(mbps));
    const int mode = mode_combo->currentData().toInt();
    if (!camera_manager->SetColorCompression(selected_serial, mode, quality, bitrateScaled)) {
        emit showWarning("Unsupported Camera", "Color compression is only supported on Prime Color cameras.");
    }
}

void CameraControlPanel::onSetVideoMode(int modeEnum) {
    if (!currentSerialValid()) return;
    QString err;
    if (!camera_manager->SetVideoType(selected_serial, static_cast<Core::eVideoMode>(modeEnum), &err)) {
        if (!err.isEmpty()) emit showWarning("Unsupported Mode", err);
    }
}
